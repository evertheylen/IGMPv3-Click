#include <string>
#include <algorithm>
#include <random>

#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include <click/timer.hh>

#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <clicknet/udp.h>

#include "ClientGroupState.hh"
#include "MCTable.hh"
#include "ClientIGMP.hh"
#include "constants.hh"
#include "util.hh"

CLICK_DECLS

// Handling of messages
// ====================

void ClientIGMP::got_query(int port, Query* query, Packet* p) {
	// 5.2.1 + 5.2.2
	unsigned int delay = random_ms();
	if (general_query_timer.scheduled() and left(general_query_timer) < delay) {
		click_chatter("%s: \tgot (General) Query, case 5.2.1 (do nothing)\n", name().c_str());
	} else if (query->group_address == IPAddress(0)) {
		click_chatter("%s: \tgot (General) Query, case 5.2.2 (schedule GQ resp)\n", name().c_str());
		general_query_timer.schedule_after_msec(delay);
	} else {
		ClientSubTable& st = table->local();
		auto it = st.table.find(query->group_address);
		if (it == st.table.end()) return;
		ClientGroupState& gs = it->second;
		
		if (not gs.current_state_timer.scheduled()) { // 5.2.3
			click_chatter("%s: \tgot Query, case 5.2.3\n", name().c_str());
			if (query->N != 0) {
				auto _sources = query->sources();
				gs.sources_to_report.insert(_sources.begin(), _sources.end());
			}
			gs.current_state_timer.schedule_after_msec(delay);
		} else {
			if (query->N == 0 or gs.sources_to_report.empty()) { // 5.2.4
				click_chatter("%s: \tgot Query, case 5.2.4\n", name().c_str());
				gs.sources_to_report.clear();
			} else { // 5.2.5
				click_chatter("%s: \tgot Query, case 5.2.5\n", name().c_str());
				auto _sources = query->sources();
				gs.sources_to_report.insert(_sources.begin(), _sources.end());
			}
			if (delay < left(gs.current_state_timer))
				gs.current_state_timer.schedule_after_msec(delay);
		}
	}
}

unsigned int ClientIGMP::random_ms() {
	std::uniform_int_distribution<unsigned int> dist(0, max_resp_time*100);
	return dist(rd);
}

void ClientIGMP::run_timer(Timer* timer) {
	// p23, below
	ClientSubTable& subtable = table->local();
	int size = subtable.table.size();
	if (size > 0) {
		ReportBuilder rb;
		for (auto& gs_it: subtable.table) {
			rb.add_record(MODE_IS_(gs_it.second.include), gs_it.first, gs_it.second.sources);
		}
		rb.prepare();
		output(LOCAL).push(rb.new_packet());
	}
}



// Configuration and handlers
// ==========================

int ClientIGMP::initialize(ErrorHandler *errh) {
	general_query_timer.initialize(this, true);
	general_query_timer.assign(this);
	return 0;
}

void ClientIGMP::change_mode(const String& s, bool silent) {
	// See 5.1
	std::vector<String> parts = split(s, ' ');
	
	if (parts.size() < 2) {
		click_chatter("%s: \tGot invalid change_mode handler request: not enough arguments\n", name().c_str());
		return;
	}
	
	bool include;
	if (parts[0] == String("INCLUDE")) {
		include = true;
	} else if (parts[0] == String("EXCLUDE")) {
		include = false;
	} else {
		click_chatter("%s: \tGot invalid change_mode handler request: wrong filter-mode\n", name().c_str());
		return;
	}
	
	IPAddress group(parts[1]);
	
	std::vector<IPAddress> sources(parts.size()-2);
	for (int i=2; i<parts.size(); i++) {
		sources[i-2] = IPAddress(parts[i]);
	}
	
	ClientGroupState& gs = table->local().get(group);
	gs.change_to(include, sources, silent);
}

void ClientIGMP::change_sources(const String& s, bool silent) {
	std::vector<String> parts = split(s, ' ');
	if (parts.size() <= 1) {
		click_chatter("%s: \tGot invalid change_sources handler request: not enough arguments\n", name().c_str());
		return;
	} else if (parts.size() <= 2) {
		click_chatter("%s: \tIgnoring change_sources call because of empty sources\n", name().c_str());
		return;
	}
	
	bool allow;
	if (parts[0] == String("ALLOW")) {
		allow = true;
	} else if (parts[0] == String("BLOCK")) {
		allow = false;
	} else {
		click_chatter("%s: \tGot invalid change_sources handler request: wrong argument (ALLOW/BLOCK)\n", name().c_str());
		return;
	}
	
	IPAddress group(parts[1]);
	
	std::vector<IPAddress> sources(parts.size()-2);
	for (int i=2; i<parts.size(); i++) {
		sources[i-2] = IPAddress(parts[i]);
	}
	
	ClientGroupState& gs = table->local().get(group);
	gs.change_sources(allow, sources, silent);
}

void ClientIGMP::add_handlers() {
	add_write_handler("change_mode", change_mode_handler, 0);
	add_write_handler("silent_change_mode", change_mode_handler, 1);
	add_write_handler("change_sources", change_sources_handler, 0);
	add_write_handler("silent_change_sources", change_sources_handler, 1);
}

int ClientIGMP::change_mode_handler(const String &s, Element* e, void* silent, ErrorHandler* errh) {
	((ClientIGMP*) e)->change_mode(s, bool(silent));
	return 0;
}

int ClientIGMP::change_sources_handler(const String &s, Element* e, void* silent, ErrorHandler* errh) {
	((ClientIGMP*) e)->change_sources(s, bool(silent));
	return 0;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(ClientIGMP)

// Well thanks click for not including my C++ files
#include "util.cc"
#include "Report.cc"
#include "Query.cc"
#include "IGMP.cc"
#include "GroupState.cc"
#include "ClientGroupState.cc"
#include "RouterGroupState.cc"
