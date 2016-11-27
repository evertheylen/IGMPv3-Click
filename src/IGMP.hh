
#pragma once

#include <map>

#include <click/element.hh>

#include "MulticastTable.hh"
#include "Query.hh"
#include "Report.hh"

CLICK_DECLS

// IGMP does the the management part (which should be obvious since IGMP stands for
// Internet Group Management Protocol). Queries in IGMP are only sent to all
// interfaces, so this needs only one output port. However, we do need to know
// from which interface this message came, so it has an input port for each
// interface
class IGMP: public Element {
public:
	const char *class_name() const	{ return "IGMP"; }
	const char *port_count() const	{ return "1/1"; }
	const char *processing() const	{ return PUSH; }
	void add_handlers();

	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet*);
	void got_report(int interface, Report* report, Packet* p);
	void got_query(int interface, Query* query, Packet* p);

	// Host is always assumed to be port 0
	void host_update(bool include, bool silent, const String& s);
	
	inline centiseconds GMI() { return (robustness*query_interval)*10 + max_resp_time; }

protected:
	MulticastTable* table;
	
	uint8_t robustness = defaults::ROBUSTNESS;
	seconds query_interval = defaults::QUERY_INTERVAL;
	centiseconds max_resp_time = defaults::MAX_RESP_TIME;
	
	static int join_group_handler(const String &s, Element* e, void* silent, ErrorHandler* errh);
	static int leave_group_handler(const String &s, Element* e, void* silent, ErrorHandler* errh);
};

CLICK_ENDDECLS
