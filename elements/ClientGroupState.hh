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

class ClientMCTable;

using _ClientGroupState = GroupState<ClientMCTable>;

class ClientGroupState: public _ClientGroupState {
public:
	std::set<IPAddress> sources;
	unsigned int retransmit_mode = 0;
	std::map<IPAddress, RetransmitState> retransmit_sources;
	Timer retransmit_timer;
	
	ClientGroupState(void* _table = nullptr, int _interface = -1, IPAddress _group = 0);
	ClientGroupState(const ClientGroupState& other);
	ClientGroupState& operator=(const ClientGroupState&);
	void init_timers();
	
	template <typename Iterable>
	void change_to(bool _include, Iterable _sources, bool silent=false);
	
	template <typename Iterable>
	void change_sources(bool allow, Iterable _sources, bool silent=false);
	
	bool forward(IPAddress source);
	
	bool is_default() const;
	
	static void run_robustness_timer(Timer* t, void* user_data);
	void timer_expired();
	GroupRecord* add_filter_mode_record(ReportBuilder& rb);
	
	// debugging
	std::string description();
	
	static const ClientGroupState DEFAULT;
	
protected:
	unsigned int random_ms();
};

