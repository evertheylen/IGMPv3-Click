#include <click/config.h>
#include <click/error.hh>
#include <click/args.hh>

#include "Multicast.hh"

CLICK_DECLS

int Multicast::configure(Vector<String>& conf, ErrorHandler* errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("MulticastTable"), table)
		.consume() < 0)
		return -1;
	num_interfaces = noutputs();
	return 0;
}

void Multicast::push(int, Packet* p) {
	bool pushed = false;
	for (int i=0; i<num_interfaces; i++) {
		if (table->get(i, p->dst_ip_anno())) {
			pushed = true;
			output(i).push(p->clone());
		}
	}
	
	if (not pushed) p->kill();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(Multicast)
