
#pragma once

#include <click/element.hh>

#include "MulticastTable.hh"

CLICK_DECLS

// Multicast does the actual multicasting. There's only one input (logically),
// but for every interface there is an output.
class Multicast: public Element {
public:
	const char *class_name() const	{ return "Multicast"; }
	const char *port_count() const	{ return "1/1-"; } // one for each interface. The first one is the local one
	const char *processing() const	{ return PUSH; }
	
	int configure(Vector<String>& conf, ErrorHandler* errh);
	
	void push(int, Packet*);
// 	Packet* pull(int);

private:
	int num_interfaces;
	MulticastTable* table;
};

CLICK_ENDDECLS
