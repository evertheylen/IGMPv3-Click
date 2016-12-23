#include <click/config.h>
#include <click/error.hh>
#include <click/args.hh>

#include "MC.hh"

CLICK_DECLS

void MC::push(int, Packet* p) {
	bool pushed = false;
	for (int i=0; i<noutputs(); i++) {
		if (get_table()->get(i, p->dst_ip_anno(), p->ip_header()->ip_src)) {
			pushed = true;
			output(i).push(p->clone());
		}
	}
	
	if (not pushed) p->kill();
}

int ClientMC::configure(Vector<String>& conf, ErrorHandler* errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("ClientMCTable"), table)
		.consume() < 0)
		return -1;
	return 0;
}

int RouterMC::configure(Vector<String>& conf, ErrorHandler* errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("RouterMCTable"), table)
		.consume() < 0)
		return -1;
	return 0;
}

CLICK_ENDDECLS

EXPORT_ELEMENT(ClientMC)
EXPORT_ELEMENT(RouterMC)
