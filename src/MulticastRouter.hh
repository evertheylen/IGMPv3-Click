
#pragma once

#include <click/element.hh>

#include "MulticastRouterTable.hh"

CLICK_DECLS

class MulticastRouter: public Element {
public:
	MulticastRouter();
	
	const char *class_name() const	{ return "MulticastRouter"; }
	const char *port_count() const	{ return "1/1"; }
	const char *processing() const	{ return AGNOSTIC; }
	
	int configure(Vector<String>& conf, ErrorHandler* errh);
	
	void push(int, Packet *);
	Packet* pull(int);

private:
	MulticastRouterTable* table;
};

CLICK_ENDDECLS
