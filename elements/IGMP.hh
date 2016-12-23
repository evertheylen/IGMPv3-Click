
#pragma once

#include <click/config.h>
#include <click/element.hh>

#include "Query.hh"
#include "Report.hh"
#include "constants.hh"

class IGMP: public Element {
public:
	const char *class_name() const	{ return "_IGMP"; }
	const char *processing() const	{ return PUSH; }
	
	void push(int, Packet*);
	virtual void got_report(int port, Report* report, Packet* p);
	virtual void got_query(int port, Query* query, Packet* p);
	
	inline centiseconds GMI() { return (robustness*query_interval)*10 + max_resp_time; }
	inline centiseconds LMQT() {
		return last_member_query_count * last_member_query_interval;
	}
	
protected:
	uint8_t robustness = defaults::ROBUSTNESS;
	seconds query_interval = defaults::QUERY_INTERVAL;
	centiseconds max_resp_time = defaults::MAX_RESP_TIME;
	centiseconds last_member_query_interval = defaults::LAST_MEMBER_QUERY_INTERVAL;
	unsigned int last_member_query_count = defaults::LAST_MEMBER_QUERY_COUNT;
};