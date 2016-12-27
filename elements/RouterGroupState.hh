
#pragma once

#include <map>
#include <memory>

#include <click/ipaddress.hh>
#include <click/timer.hh>

#include "GroupState.hh"
#include "Report.hh"
#include "Query.hh"

class RouterGroupState;

class SourceTimer {
public:
	RouterGroupState* gs;
	IPAddress source;
	Timer timer;
	
	SourceTimer(RouterGroupState* _gs, IPAddress _source, unsigned int milliseconds = 0);
};

class RouterMCTable;

using _RouterGroupState = GroupState<RouterMCTable>;

class RouterGroupState: public _RouterGroupState {
public:
	Timer group_timer; // only used in EXCLUDE
	std::map<IPAddress, SourceTimer*> sources;
	
	RouterGroupState(void* _table = nullptr, int _interface = -1, IPAddress _group = 0);
	RouterGroupState(const RouterGroupState& other);
	RouterGroupState& operator=(const RouterGroupState&);
	void init_timers();
	~RouterGroupState();
	
	bool is_default() const;
	
	bool forward(IPAddress source) const;
	
	// fired on group_timer expiry
	static void run_group_timer(Timer* t, void* user_data);
	void group_timer_expired();
	
	// fired on source timer expiry
	static void run_source_timer(Timer* t, void* user_data);
	void source_timer_expired(SourceTimer* source_timer);
	
	void got_current_state_record(GroupRecord* record);
	void got_state_change_record(GroupRecord* record);
	
	// debugging
	std::string description();
	
protected:
	// Utitilities for implementing table actions:
	void schedule_source(IPAddress ip, unsigned int milliseconds);
	
	// Abbreviations in tables
	void Q();  // General Query
	
	template <typename Iterable>
	void Q(Iterable A);// Group-Specific Query	
	
	milliseconds group_timer_ms();
	
	static const RouterGroupState DEFAULT;
};

