
#pragma once

#include <map>

#include <click/element.hh>

#include "MCTable.hh"
#include "Query.hh"
#include "Report.hh"
#include "IGMP.hh"

CLICK_DECLS

// IGMP does the the management part.
class ClientIGMP: public IGMP {
public:
	const char *class_name() const	{ return "ClientIGMP"; }
	const char *port_count() const	{ return "1/1"; }
	void add_handlers();
	
	int configure(Vector<String>&, ErrorHandler*);
	
	void got_query(int port, Query* query, Packet* p);
	
	void change_mode(const String& s, bool silent);
	void change_sources(const String& s, bool allow);
	
protected:
	ClientMCTable* table;
	
	static int change_mode_handler(const String &s, Element* e, void* silent, ErrorHandler* errh);
	static int change_sources_handler(const String &s, Element* e, void* silent, ErrorHandler* errh);
};

CLICK_ENDDECLS
