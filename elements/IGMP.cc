
#include "IGMP.hh"
#include "Query.hh"
#include "Report.hh"

void IGMP::push(int port, Packet* p) {
	const MessageType type = (MessageType) *p->data();
	if (type == MessageType::QUERY) {
		if (not checksum_ok<Query>(p)) return;
		got_query(port, (Query*) p->data(), p);
	} else if (type == MessageType::REPORT) {
		if (not checksum_ok<Report>(p)) return;
		got_report(port, (Report*) p->data(), p);
	} else {
		click_chatter("%s: \tIGMP got a message with an unknown type\n", name().c_str());
	}
	p->kill();
}

void IGMP::got_query(int port, Query* query, Packet* p) {
	click_chatter("%s: \tgot unexpected query\n", name().c_str());
}

void IGMP::got_report(int port, Report* report, Packet* p) {
	click_chatter("%s: \tgot unexpected report\n", name().c_str());
}


