
#pragma once

#include <click/element.hh>

#include "MulticastRouterTable.hh"

CLICK_DECLS

class IGMPRouter: public Element {
public:
	IGMPRouter();
	
	const char *class_name() const	{ return "IGMPRouter"; }
	const char *port_count() const	{ return "1/1"; }
	const char *processing() const	{ return AGNOSTIC; }
	int configure(Vector<String>&, ErrorHandler*);
	
	Packet* simple_action(Packet* p);
	
// 	void push(int, Packet *);
// 	Packet* pull(int);

private:
	MulticastRouterTable* table;
};

CLICK_ENDDECLS
