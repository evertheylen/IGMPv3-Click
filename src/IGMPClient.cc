#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "IGMPClient.hh"

CLICK_DECLS
IGMPClient::IGMPClient() {}


int IGMPClient::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (conf.size() > 0) return errh->error("Only specify an empty configuration string");
	return 0;
}

void IGMPClient::push(int, Packet *p){
	output(0).push(p);
}

Packet* IGMPClient::pull(int) {
	return input(0).pull();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPClient)
