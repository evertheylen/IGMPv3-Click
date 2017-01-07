
#pragma once

#include <click/config.h>
#include <click/element.hh>

#include "Query.hh"
#include "Report.hh"
#include "constants.hh"

class MCTable;

class IGMP: public Element {
public:
	int parent = -1; // Interface where the parent router can be found. Important for Routers as members.
	MCTable* table = nullptr;
	
	const char *class_name() const	{ return "_IGMP"; }
	const char *processing() const	{ return PUSH; }
	
	int configure(Vector<String>& conf, ErrorHandler* e);
	int initialize(ErrorHandler* errh);
	
	void push(int, Packet*);
	virtual void got_report(int port, Report* report, Packet* p);
	virtual void got_query(int port, Query* query, Packet* p);
	
	inline centiseconds GMI() { return (robustness*query_interval)*10 + max_resp_time; }
	inline centiseconds LMQT() { return last_member_query_count * last_member_query_interval; }
	inline seconds URI() { return unsolicited_report_interval; }
	inline uint8_t get_robustness() { return robustness; }
	
protected:
	Timer respond_to_gq_timer;
	static void run_respond_to_gq_timer(Timer* timer, void* user_data);
	unsigned int random_ms();
	
	uint8_t robustness = defaults::ROBUSTNESS;
	seconds query_interval = defaults::QUERY_INTERVAL;
	centiseconds max_resp_time = defaults::MAX_RESP_TIME;
	centiseconds last_member_query_interval = defaults::LAST_MEMBER_QUERY_INTERVAL;
	unsigned int last_member_query_count = defaults::LAST_MEMBER_QUERY_COUNT;
	seconds unsolicited_report_interval = defaults::UNSOLICITED_REPORT_INTERVAL;
};
