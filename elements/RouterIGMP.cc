#include <click/config.h>
#include <click/error.hh>
#include <click/args.hh>

#include "RouterIGMP.hh"
#include "RouterGroupState.hh"

CLICK_DECLS

int RouterIGMP::initialize(ErrorHandler* errh) {
	IGMP::initialize(errh);
	send_gq_timer.initialize(this, true);
	send_gq_timer.assign(RouterIGMP::run_send_gq_timer, this);
	send_gq_timer.schedule_now();
	return 0;
}

void RouterIGMP::run_send_gq_timer(Timer*, void* user_data) {
	RouterIGMP* r = (RouterIGMP*) user_data;
	click_chatter("%s: \tSending general query", r->name().c_str());
	QueryBuilder qb(0); // 0 --> General Query
	qb.query()->QRV = r->robustness;
	qb.prepare();
	for (int i=0; i<r->noutputs(); i++) {
		if (i != r->parent) r->output(i).push(qb.new_packet());
	}
	r->send_gq_timer.reschedule_after_sec(r->query_interval);
}

void RouterIGMP::got_report(int port, Report* report, Packet* p) {
	if (port == parent) return;
	uint16_t N = ntoh_16(report->number_group_records);

	GroupRecord* record = (GroupRecord*) (p->data() + sizeof(Report));
	for (int i=0; i<N; i++) {
		RouterGroupState& gs = table->router(port).get(record->multicast_address);
		click_chatter("%s: \tRouterIGMP port %d: got a Record:       %s", name().c_str(), port, record->description().c_str());
		click_chatter("%s: \tRouterIGMP port %d: current GroupState: %s", name().c_str(), port, gs.description().c_str());
		if (is_current_state(record->type)) {
			gs.got_current_state_record(record);
		} else if (is_state_change(record->type)) {
			gs.got_state_change_record(record);
		}
		click_chatter("%s: \tRouterIGMP port %d: new GroupState:     %s", name().c_str(), port, gs.description().c_str());
		table->router(port).groupstate_changed(gs);
		
		record = pointer_add(record, record->size());
	}
}


CLICK_ENDDECLS
EXPORT_ELEMENT(RouterIGMP)
