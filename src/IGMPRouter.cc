#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>

#include "IGMPRouter.hh"
#include "Query.hh"
#include "Report.hh"
#include "constants.hh"

CLICK_DECLS
IGMPRouter::IGMPRouter() {}


int IGMPRouter::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("MulticastRouterTable"), table)
		.consume() < 0)
		return -1;
	
	return 0;
}

Packet* IGMPRouter::simple_action(Packet* p) {
	const MessageType type = (MessageType) *p->data();
	if (type == MessageType::QUERY) {
		Query* query = (Query*) p->data();
		click_chatter("got a query\n");
	} else {
		Report* report = (Report*) p->data();
		click_chatter("got a report, turn online is %d\n", report->turn_online);
		table->online = report->turn_online;
	}
	return p;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPRouter)
