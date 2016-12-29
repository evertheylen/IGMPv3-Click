#include <click/config.h>
#include <click/error.hh>
#include <click/args.hh>

#include "MC.hh"
#include "ClientGroupState.hh"

CLICK_DECLS


int MC::configure(Vector<String>& conf, ErrorHandler* errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("MCTable"), table)
		.consume() < 0)
		return -1;
	return 0;
}

void ClientMC::push(int, Packet* p) {
	click_chatter("CLIEEEEEEEEEEEEEENT\n");
	auto it = table->local().table.find(p->dst_ip_anno());
	if (it == table->local().table.end()) {
		p->kill();
		return;
	}
		
	bool forward = it->second.forward(p->ip_header()->ip_src);
	click_chatter("%s: \tClient state = %s --> %s\n", name().c_str(), 
				  it->second.description().c_str(), forward ? "Go along..." : "DENIED");
	if (forward) output(0).push(p);
	else p->kill();
	click_chatter("CLIIIIIIIIIIIIIIIIIIEEENT DONE\n");
}

void RouterMC::push(int, Packet* p) {
	bool pushed = false;
	for (int i=0; i<noutputs(); i++) {
		if (i == table->igmp->parent) continue;
		auto subtable = table->router(i).table;
		auto it = subtable.find(p->dst_ip_anno());
		if (it == subtable.end()) continue;
			
		bool forward = it->second.forward(p->ip_header()->ip_src);
		click_chatter("%s: \tRouter state = %s --> %s\n", name().c_str(), 
					it->second.description().c_str(), forward ? "Go along..." : "DENIED");
		if (forward) {
			output(i).push(p->clone());
			pushed = true;
		}
	}
	
	if (not pushed) p->kill();
}

CLICK_ENDDECLS

EXPORT_ELEMENT(ClientMC)
EXPORT_ELEMENT(RouterMC)
