#include <click/config.h>
#include <click/error.hh>
#include <click/args.hh>

#include "MulticastRouter.hh"

CLICK_DECLS

MulticastRouter::MulticastRouter() {}

int MulticastRouter::configure(Vector<String>& conf, ErrorHandler* errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("MulticastRouterTable"), table)
		.consume() < 0)
		return -1;
	
	return 0;
}

void MulticastRouter::push(int, Packet *p) {
	if (table->online)
		output(0).push(p);
	else
		p->kill();
}

Packet* MulticastRouter::pull(int) {
	if (table->online)
		return input(0).pull();
	else
		return nullptr;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(MulticastRouter)
