#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>

#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <clicknet/udp.h>

#include "IGMP.hh"

#include "constants.hh"
#include "util.hh"

CLICK_DECLS

// Handling of messages
// ====================

void IGMP::push(int port, Packet* p) {
	const MessageType type = (MessageType) *p->data();
	int interface = port + 1; // important: the first interface is the local one (on which we don't receive messages)
	if (type == MessageType::QUERY) {
		got_query(interface, (Query*) p->data(), p);
	} else if (type == MessageType::REPORT) {
		got_report(interface, (Report*) p->data(), p);
	} else {
		click_chatter("IGMP got a message with an unknown type\n");
	}
	p->kill();
}

void IGMP::got_report(int interface, Report* report, Packet* p) {
	uint16_t N = ntoh_16(report->number_group_records);
	
	GroupRecord* record = (GroupRecord*) (p->data() + sizeof(Report));
	for (int i=0; i<N; i++) {
		table->set(interface, record->multicast_address, record->include());
		record += record->size();
	}
}

void IGMP::got_query(int interface, Query* query, Packet* p) {
	
}


// Configuration and handlers
// ==========================

int IGMP::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("MulticastTable"), table)
		.consume() < 0)
		return -1;
	
	return 0;
}

void IGMP::host_update(bool include, const String& s) {
	if (include) click_chatter("sending report with join group record\n");
	else click_chatter("sending report with leave group record\n");
	
	IPAddress group(s);
	
	// tell the router
	ReportBuilder rb(1);
	rb.add_record(CHANGE_TO_(include), group);
	rb.packet->set_dst_ip_anno(REPORT_ADDRESS);
	output(0).push(rb.packet);
	
	// set own table
	table->set(0, group, include);
}

void IGMP::add_handlers() {
	add_write_handler("join_group", join_group_handler, nullptr);
	add_write_handler("leave_group", leave_group_handler, nullptr);
}

int IGMP::join_group_handler(const String &s, Element* e, void*, ErrorHandler* errh) {
	((IGMP*) e)->host_update(true, s);
	return 0;
}

int IGMP::leave_group_handler(const String &s, Element* e, void*, ErrorHandler* errh) {
	((IGMP*) e)->host_update(false, s);
	return 0;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMP)
