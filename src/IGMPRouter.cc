#include <click/config.h>
#include <click/error.hh>

#include "IGMPRouter.hh"

CLICK_DECLS

int IGMPRouter::initialize(ErrorHandler* errh) {
	timer.initialize(this);
	timer.assign(this);
	timer.schedule_now();
	return 0;
}

void IGMPRouter::run_timer(Timer*) {
	QueryBuilder qb(0); // 0 --> General Query
	qb.set_checksum();
	qb.packet->set_dst_ip_anno(GENERAL_QUERY_ADDRESS);
	output(0).push(qb.packet);
	timer.reschedule_after_sec(query_interval);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPRouter)
