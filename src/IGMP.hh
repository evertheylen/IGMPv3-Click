
#pragma once

#include <click/element.hh>

#include "MulticastTable.hh"

CLICK_DECLS

// IGMP does the the management part (which should be obvious since IGMP stands for 
// Internet Group Management Protocol). Queries in IGMP are only sent to all
// interfaces, so this needs only one output port. However, we do need to know
// from which interface this message came, so it has an input port for each
// interface
class IGMP: public Element {
public:
	const char *class_name() const	{ return "IGMP"; }
	const char *port_count() const	{ return "-/1"; }
	const char *processing() const	{ return PUSH; }
	void add_handlers();
	
	int configure(Vector<String>&, ErrorHandler*);
	
	void push(int, Packet*);
	
	// Host is always assumed to be port 0
	void host_update(bool new_state);
	
	//Packet* simple_action(Packet* p);
	//Packet* pull(int);

private:
	MulticastTable* table;
	
	static int join_group_handler(const String &s, Element* e, void*, ErrorHandler* errh);
	static int leave_group_handler(const String &s, Element* e, void*, ErrorHandler* errh);
};

CLICK_ENDDECLS
