
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
	void push(int, Packet*);
	virtual BaseMCTable* get_table() = 0;
};

class ClientMC: public MC {
public:
	const char *class_name() const	{ return "ClientMC"; }
	const char *port_count() const	{ return "1/1"; }
	int configure(Vector<String>& conf, ErrorHandler* errh);
	inline BaseMCTable* get_table() { return table; }
private:
	ClientMCTable* table;
};

class RouterMC: public MC {
public:
	const char *class_name() const	{ return "RouterMC"; }
	const char *port_count() const	{ return "1/-"; }
	int configure(Vector<String>& conf, ErrorHandler* errh);
	inline BaseMCTable* get_table() { return table; }
private:
	RouterMCTable* table;
};

CLICK_ENDDECLS
