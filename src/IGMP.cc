#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include <click/timer.hh>

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
	click_chatter("got report with %d records\n", N);

	GroupRecord* record = (GroupRecord*) (p->data() + sizeof(Report));
	for (int i=0; i<N; i++) {
		click_chatter("report is of size %d\n", record->size());
		table->set(interface, record->multicast_address, record->include());
		pointer_add(record, record->size());
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
	rb.set_checksum();
	output(0).push(rb.packet);

	// set own table
	table->set(0, group, include);
}

void IGMP::add_handlers() {
	add_write_handler("join_group", join_group_handler, nullptr);
	add_write_handler("leave_group", leave_group_handler, nullptr);
}

int IGMP::join_group_handler(const String &s, Element* e, void*, ErrorHandler* errh) {
	((IGMP*) e)->host_update(EXCLUDE, s);
	return 0;
}

int IGMP::leave_group_handler(const String &s, Element* e, void*, ErrorHandler* errh) {
	((IGMP*) e)->host_update(INCLUDE, s);
	return 0;
}

//Membership query


Packet * IGMP::make_membership_query() {
		WritablePacket *q = Packet::make(sizeof(click_ip) + sizeof(struct igmp_packet);// + _data.length());
		if (!q) {
				return 0;
		}
		//TODO bij send pings gebeuren er hierna nog dingen, kijken wat wij nog moeten doen voor igmp
		return q;
}

void IGMP::run_timer(Timer *) {
	//loopt de timer automatisch door <click/timer.hh>? -->nakijken hoe het reageert
    if (Packet *q = make_packet()) {
	output(0).push(q);
	_timer.reschedule_after_msec(_interval);
	/*_count++;
	if (_count < _limit || _limit < 0)
}*/
}


// Well thanks click for not including my C++ files
#include "util.cc"
#include "Report.cc"
#include "Query.cc"

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMP)
