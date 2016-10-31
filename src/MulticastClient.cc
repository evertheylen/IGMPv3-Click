#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "MulticastClient.hh"

CLICK_DECLS
MulticastClient::MulticastClient() {}


int MulticastClient::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (conf.size() > 0) return errh->error("Only specify an empty configuration string");
	return 0;
}

void MulticastClient::push(int, Packet *p){
	output(0).push(p);
}

Packet* MulticastClient::pull(int) {
	return input(0).pull();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(MulticastClient)
