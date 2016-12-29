#pragma once

#include <map>
#include <memory>

#include <click/ipaddress.hh>
#include <click/timer.hh>

#include "GroupState.hh"
#include "Report.hh"
#include "Query.hh"

struct RetransmitState {
	unsigned int transmissions = 0;
	bool allow = true;
	inline RetransmitState(unsigned int t=0, bool a=true): transmissions(t), allow(a) {}
};

class ClientGroupState: public GroupState {
public:
	std::set<IPAddress> sources;
	
	// retransmitting change-state records
	unsigned int retransmit_mode = 0;
	std::map<IPAddress, RetransmitState> retransmit_sources;
	Timer retransmit_timer;
	
	// transmitting current-state records
	std::set<IPAddress> sources_to_report;
	Timer current_state_timer;
	
	
	// init
	ClientGroupState(MCTable* _table = nullptr, IPAddress _group = 0);
	ClientGroupState(const ClientGroupState& other);
	ClientGroupState& operator=(const ClientGroupState&);
	void init_timers();
	
	// changes and reporting changes
	template <typename Iterable>
	void change_to(bool _include, Iterable _sources, bool silent=false);
	template <typename Iterable>
	void change_sources(bool allow, Iterable _sources, bool silent=false);
	
	static void run_retransmit_timer(Timer* t, void* user_data);
	void retransmit_timer_expired();
	GroupRecord* add_filter_mode_record(ReportBuilder& rb);
	
	// reporting current-state
	static void run_current_state_timer(Timer* t, void* user_data);
	void current_state_timer_expired();
	
	// basics
	bool forward(IPAddress source) const;
	
	bool is_default() const;
	
	// debugging
	std::string description();
	
	static const ClientGroupState DEFAULT;
	
protected:
	unsigned int random_ms();
};

