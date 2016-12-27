#include <click/config.h>
#include <click/error.hh>
#include <click/args.hh>

#include "RouterIGMP.hh"
#include "RouterGroupState.hh"

CLICK_DECLS

int RouterIGMP::initialize(ErrorHandler* errh) {
	timer.initialize(this, true);
	timer.assign(this);
	timer.schedule_now();
	return 0;
}

void RouterIGMP::run_timer(Timer*) {
	click_chatter("%s: \tSending general query", name().c_str());
	QueryBuilder qb(0); // 0 --> General Query
	qb.prepare();
	for (int i=0; i<noutputs(); i++) output(i).push(qb.new_packet());
	timer.reschedule_after_sec(query_interval);
}

void RouterIGMP::got_report(int port, Report* report, Packet* p) {
	uint16_t N = ntoh_16(report->number_group_records);

	GroupRecord* record = (GroupRecord*) (p->data() + sizeof(Report));
	for (int i=0; i<N; i++) {
		RouterGroupState& gs = table->get_groupstate(port, record->multicast_address);
		click_chatter("%s: \tRouterIGMP port %d: got a Record:       %s", name().c_str(), port, record->description().c_str());
		click_chatter("%s: \tRouterIGMP port %d: current GroupState: %s", name().c_str(), port, gs.description().c_str());
		if (is_current_state(record->type)) {
			gs.got_current_state_record(record);
		} else if (is_state_change(record->type)) {
			gs.got_state_change_record(record);
		}
		click_chatter("%s: \tRouterIGMP port %d: new GroupState:     %s", name().c_str(), port, gs.description().c_str());
		
		if (gs.is_default()) {
			table->get_subtable(port).erase(record->multicast_address);
		}
		
		record = pointer_add(record, record->size());
	}
}


int RouterIGMP::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("RouterMCTable"), table)
		.consume() < 0)
		return -1;
	table->set_igmp(this);
	return 0;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(RouterIGMP)
