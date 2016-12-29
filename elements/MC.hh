
#pragma once

#include <click/element.hh>

#include "MCTable.hh"

CLICK_DECLS

// Multicast does the actual multicasting. There's only one input (logically),
// but for every interface there is an output.
class MC: public Element {
public:
	const char *class_name() const	{ return "_MC"; }
	const char *processing() const	{ return PUSH; }
	int configure(Vector<String>& conf, ErrorHandler* errh);
protected:
	MCTable* table;
};

class ClientMC: public MC {
public:
	const char *class_name() const	{ return "ClientMC"; }
	const char *port_count() const	{ return "1/1"; }
	void push(int, Packet*);
};

class RouterMC: public MC {
public:
	const char *class_name() const	{ return "RouterMC"; }
	const char *port_count() const	{ return "1/-"; }
	void push(int, Packet*);
};

CLICK_ENDDECLS
