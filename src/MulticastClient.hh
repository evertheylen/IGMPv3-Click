
#pragma once

#include <click/element.hh>
CLICK_DECLS

class MulticastClient: public Element {
public:
	MulticastClient();
	
	const char *class_name() const	{ return "MulticastClient"; }
	const char *port_count() const	{ return "1/1"; }
	const char *processing() const	{ return AGNOSTIC; }
	int configure(Vector<String>&, ErrorHandler*);
	
	void push(int, Packet *);
	Packet* pull(int);
};

CLICK_ENDDECLS
