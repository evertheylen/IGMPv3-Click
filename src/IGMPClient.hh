
#pragma once

#include <click/element.hh>
CLICK_DECLS

class IGMPClient: public Element {
	public:
		IGMPClient();
		
		const char *class_name() const	{ return "IGMPClient"; }
		const char *port_count() const	{ return "1/1"; }
		const char *processing() const	{ return AGNOSTIC; }
		int configure(Vector<String>&, ErrorHandler*);
		
		void push(int, Packet *);
		Packet* pull(int);
};

CLICK_ENDDECLS
