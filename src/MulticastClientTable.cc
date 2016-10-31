#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "MulticastClientTable.hh"

CLICK_DECLS
MulticastClientTable::MulticastClientTable() {}


int MulticastClientTable::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (conf.size() > 0) return errh->error("Only specify an empty configuration string");
	return 0;
}

void MulticastClientTable::push(int, Packet *p){
	output(0).push(p);
}

Packet* MulticastClientTable::pull(int) {
	return input(0).pull();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(MulticastClientTable)
