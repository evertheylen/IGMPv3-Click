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

// Configuration and handlers
// ==========================

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
