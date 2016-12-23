#include <string>
#include <algorithm>

#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include <click/timer.hh>

#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <clicknet/udp.h>

#include "ClientIGMP.hh"

#include "constants.hh"
#include "util.hh"

CLICK_DECLS

// Handling of messages
// ====================

void ClientIGMP::got_query(int port, Query* query, Packet* p) {
	// See 5.2
	if (query->group_address == IPAddress(0)) {
		// Page 23, point 1
		click_chatter("got General Query\n");
		ClientMCTable::SubTable& subtable = table->get_subtable(LOCAL);
		int size = subtable.size();
		if (size > 0) {
			ReportBuilder rb(size);
			for (auto& gs_it: subtable) {
				rb.add_record(MODE_IS_(gs_it.second.include), gs_it.first, gs_it.second.sources);
			}
			rb.prepare();
			output(0).push(rb.packet);
		}
	} else if (query->N == 0) {
		// Page 24, point 2
		click_chatter("got Group-Specific Query\n");
		ReportBuilder rb(1);
		const ClientGroupState& gs = table->cget_groupstate(LOCAL, query->group_address);
		if (not gs.is_default()) {
			rb.add_record(MODE_IS_(gs.include), query->group_address, gs.sources);
			rb.prepare();
			output(0).push(rb.packet);
		}
	} else {
		// Page 24, point 3 (has a table)
		click_chatter("got Group-and-Source-Specific Query\n");
		const ClientGroupState& gs = table->cget_groupstate(LOCAL, query->group_address);
		
		// in case of include, this is A*B, otherwise it is B-A
		std::vector<IPAddress> reported_sources;
		for (IPAddress b: query->sources()) {
			bool in_A = (gs.sources.find(b) != gs.sources.end());
			if (gs.include == in_A) reported_sources.push_back(b);
		}
		
		if (not reported_sources.empty()) {
			ReportBuilder rb(1);
			rb.add_record(MODE_IS_(gs.include), query->group_address, reported_sources);
			rb.prepare();
			output(0).push(rb.packet);
		} else {
			click_chatter("no sources to report!\n");
		}
	}
}


// Configuration and handlers
// ==========================

int ClientIGMP::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("ClientMCTable"), table)
		.consume() < 0)
		return -1;
	table->set_igmp(this);
	return 0;
}

void ClientIGMP::change_mode(const String& s, bool silent) {
	// See 5.1
	std::vector<String> parts = split(s, ' ');
	
	if (parts.size() < 2) {
		click_chatter("Got invalid change_mode handler request: not enough arguments\n");
		return;
	}
	
	bool include;
	if (parts[0] == String("INCLUDE")) {
		include = true;
	} else if (parts[0] == String("EXCLUDE")) {
		include = false;
	} else {
		click_chatter("Got invalid change_mode handler request: wrong filter-mode\n");
		return;
	}
	
	IPAddress group(parts[1]);
	
	std::vector<IPAddress> sources(parts.size()-2);
	for (int i=2; i<parts.size(); i++) {
		sources[i-2] = IPAddress(parts[i]);
	}
	
	if (include) click_chatter("host%s leaves a group\n", silent? " (silently)" : "");
	else click_chatter("host%s joins a group\n", silent? " (silently)": "");

	if (not silent) {
		// tell the router
		const ClientGroupState& gs = table->get_groupstate(LOCAL, group);
		// Translation of table at page 20
		// A = old set of sources
		// B = new set of sources
		if (gs.include == include) {
			
			std::vector<IPAddress> A_B;
			for (const IPAddress& a: gs.sources)
				if (std::find(sources.begin(), sources.end(), a) == sources.end())
					A_B.push_back(a);
			
			std::vector<IPAddress> B_A;
			for (const IPAddress& b: sources)
				if (gs.sources.find(b) == gs.sources.end())
					B_A.push_back(b);
			
			if (not (B_A.empty() and A_B.empty())) {
				ReportBuilder rb(int(not B_A.empty()) + int(not A_B.empty()));
				
				if (include) {
					// ALLOW (B-A)
					if (not B_A.empty()) rb.add_record(RecordType::ALLOW_NEW_SOURCES, group, B_A);
					// BLOCK (A-B)
					if (not A_B.empty()) rb.add_record(RecordType::BLOCK_OLD_SOURCES, group, A_B);
				} else {
					// ALLOW (A-B)
					if (not A_B.empty()) rb.add_record(RecordType::ALLOW_NEW_SOURCES, group, A_B);
					// BLOCK (B-A)
					if (not B_A.empty()) rb.add_record(RecordType::BLOCK_OLD_SOURCES, group, B_A);
				}
				
				// TODO robustness
				rb.prepare();
				output(0).push(rb.packet);
			} else {
				click_chatter("No new sources for change_mode...\n");
			}
		} else {
			ReportBuilder rb(1);
			rb.add_record(CHANGE_TO_(include), group, sources);
			rb.prepare();
			output(0).push(rb.packet);
		}
	}

	// set own table
	ClientGroupState& gs = table->get_groupstate(LOCAL, group); //, not include, sources);
	gs.change_to(include, sources);
	if (gs.is_default()) table->get_subtable(LOCAL).erase(group);
}

void ClientIGMP::change_sources(const String& s, bool allow) {
	std::vector<String> parts = split(s, ' ');
	if (parts.size() == 0) {
		click_chatter("Got invalid change_mode handler request: not enough arguments\n");
		return;
	} else if (parts.size() == 1) {
		click_chatter("Ignoring change_sources call because of empty sources\n");
		return;
	}
	
	IPAddress group(parts[0]);
	
	std::vector<IPAddress> sources(parts.size()-1);
	for (int i=1; i<parts.size(); i++) {
		sources[i-1] = IPAddress(parts[i]);
	}
	
	// change_sources is never silent
	ReportBuilder rb(1);
	rb.add_record(allow ? RecordType::ALLOW_NEW_SOURCES : RecordType::BLOCK_OLD_SOURCES,
				  group, sources);
	rb.prepare();
	output(0).push(rb.packet);
	click_chatter("Sent a report to the router %s %d sources\n", allow? "allow" : "block", sources.size());
	
	// set own table
	ClientGroupState& gs = table->get_groupstate(LOCAL, group); //, not include, sources);
	gs.change_sources(allow, sources);
	if (gs.is_default()) table->get_subtable(LOCAL).erase(group);
}

void ClientIGMP::add_handlers() {
	add_write_handler("change_mode", change_mode_handler, 0);
	add_write_handler("silent_change_mode", change_mode_handler, 1);
	add_write_handler("allow", change_sources_handler, 1);
	add_write_handler("block", change_sources_handler, 0);
}

int ClientIGMP::change_mode_handler(const String &s, Element* e, void* silent, ErrorHandler* errh) {
	((ClientIGMP*) e)->change_mode(s, bool(silent));
	return 0;
}

int ClientIGMP::change_sources_handler(const String &s, Element* e, void* allow, ErrorHandler* errh) {
	((ClientIGMP*) e)->change_sources(s, bool(allow));
	return 0;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(ClientIGMP)

// Well thanks click for not including my C++ files
#include "util.cc"
#include "Report.cc"
#include "Query.cc"
#include "IGMP.cc"
