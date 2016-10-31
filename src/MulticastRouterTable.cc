#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "MulticastRouterTable.hh"

CLICK_DECLS
MulticastRouterTable::MulticastRouterTable() {}


int MulticastRouterTable::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (conf.size() > 0) return errh->error("Only specify an empty configuration string");
	return 0;
}



CLICK_ENDDECLS
EXPORT_ELEMENT(MulticastRouterTable)
