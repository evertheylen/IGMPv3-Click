#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>

#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <clicknet/udp.h>

#include "IGMP.hh"
#include "Query.hh"
#include "Report.hh"
#include "constants.hh"
#include "util.hh"

CLICK_DECLS

int IGMP::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("MulticastTable"), table)
		.consume() < 0)
		return -1;
	
	return 0;
}

void IGMP::add_handlers() {
	add_write_handler("join_group", join_group_handler, nullptr);
	add_write_handler("leave_group", leave_group_handler, nullptr);
}

void IGMP::push(int port, Packet* p) {
	const MessageType type = (MessageType) *p->data();
	if (type == MessageType::QUERY) {
		Query* query = (Query*) p->data();
		click_chatter("got a query\n");
	} else {
		Report* report = (Report*) p->data();
		click_chatter("got a report\n");
		int interface = port + 1; // important, the first interface is the local one (on which we don't receive messages)
		table->set(interface, not table->get(port));  // just switch
	}
}

void IGMP::host_update(bool new_state) {
	if (new_state) click_chatter("sending join report\n");
	else click_chatter("sending leave report\n");
	
	simple_packet(Report, r, p);
	p->set_dst_ip_anno(GENERAL_QUERY_ADDRESS);
	output(0).push(p);
	table->set(0, new_state);
}

int IGMP::join_group_handler(const String &s, Element* e, void*, ErrorHandler* errh) {
	IGMP* self = (IGMP*) e;
	self->host_update(true);
	return 0;
}

int IGMP::leave_group_handler(const String &s, Element* e, void*, ErrorHandler* errh) {
	IGMP* self = (IGMP*) e;
	self->host_update(false);
	return 0;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMP)
