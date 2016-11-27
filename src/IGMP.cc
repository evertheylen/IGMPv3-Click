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
	if (not checksum_ok<Report>(p)) return;
	
	uint16_t N = ntoh_16(report->number_group_records);
	click_chatter("got report with %d records\n", N);

	GroupRecord* record = (GroupRecord*) (p->data() + sizeof(Report));
	for (int i=0; i<N; i++) {
		table->set(interface, record->multicast_address, record->include());
		pointer_add(record, record->size());
	}
}

void IGMP::got_query(int interface, Query* query, Packet* p) {
	if (not checksum_ok<Query>(p)) return;
	
	// TODO: For now, this will only send local state
	// When using multiple routers, this should "sum" the state by all interfaces
	if (query->group_address == IPAddress(0)) {
		click_chatter("got General Query\n");
		MulticastTable::SubTable& subtable = table->get_subtable(0);
		int size = subtable.size();
		if (size > 0) {
			ReportBuilder rb(size);
			for (std::pair<const IPAddress, GroupState>& gs: subtable) {
				rb.add_record(
					MODE_IS_(gs.second.include),
					gs.first);
			}
			rb.prepare();
			output(0).push(rb.packet);
		}
	} else {
		click_chatter("got Group-Specific Query\n");
		ReportBuilder rb(1);
		rb.add_record(MODE_IS_(table->get(0, query->group_address)), query->group_address);
		rb.prepare();
		output(0).push(rb.packet);
	}
}


// Configuration and handlers
// ==========================

int IGMP::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("MulticastTable"), table)
		.consume() < 0)
		return -1;
	table->set_igmp(this);
	return 0;
}

void IGMP::host_update(bool include, bool silent, const String& s) {
	if (include) click_chatter("host%s leaves a group\n", silent? " (silently)" : "");
	else click_chatter("host%s joins a group\n", silent? " (silently)": "");

	IPAddress group(s);

	if (not silent) {
		// tell the router
		ReportBuilder rb(1);
		rb.add_record(CHANGE_TO_(include), group);
		rb.prepare();
		output(0).push(rb.packet);
	}

	// set own table
	table->set(0, group, include);
}

void IGMP::add_handlers() {
	add_write_handler("join_group", join_group_handler, 0);
	add_write_handler("leave_group", leave_group_handler, 0);
	add_write_handler("join_group_silent", join_group_handler, 1);
	add_write_handler("leave_group_silent", leave_group_handler, 1);
}

int IGMP::join_group_handler(const String &s, Element* e, void* silent, ErrorHandler* errh) {
	((IGMP*) e)->host_update(EXCLUDE, bool(silent), s);
	return 0;
}

int IGMP::leave_group_handler(const String &s, Element* e, void* silent, ErrorHandler* errh) {
	((IGMP*) e)->host_update(INCLUDE, bool(silent), s);
	return 0;
}



// Well thanks click for not including my C++ files
#include "util.cc"
#include "Report.cc"
#include "Query.cc"

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMP)
