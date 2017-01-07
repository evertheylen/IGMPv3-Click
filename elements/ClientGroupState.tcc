
#include <algorithm>

#include "ClientGroupState.hh"
#include "MCTable.hh"
#include "IGMP.hh"


template <typename Iterable>
void ClientGroupState::change_to(bool _include, Iterable _sources, bool silent) {
	if (include == _include) {
		// See page 20 (A = old, B = new)
		std::vector<IPAddress> A_B;
		std::vector<IPAddress> B_A;
		
		for (const IPAddress& a: sources)
			if (std::find(_sources.begin(), _sources.end(), a) == _sources.end())
				A_B.push_back(a);
	
		for (const IPAddress& b: _sources)
			if (sources.find(b) == sources.end())
				B_A.push_back(b);
		
		// Page 20 translates to: if INCLUDE, allow B-A and block A-B, else block B-A and allow A-B
		if (not B_A.empty()) change_sources(include, B_A, silent);
		if (not A_B.empty()) change_sources(not include, A_B, silent);
	} else {
		include = _include;
		sources.clear();
		for (IPAddress s: _sources) sources.insert(s);
		if (not silent) {
			retransmit_mode = table->igmp->get_robustness();
			retransmit_timer.schedule_now();
		}
	}
	table->local().groupstate_changed(*this);
}

template <typename Iterable>
void ClientGroupState::change_sources(bool allow, Iterable _sources, bool silent) {
	// set state
	if (include == allow) sources.insert(_sources.begin(), _sources.end());
	else for (IPAddress _s: _sources) sources.erase(_s);
	
	// tell router
	if (not silent) {
		for (IPAddress _s: _sources) {
			auto it = retransmit_sources.find(_s);
			if (it == retransmit_sources.end()) {
				retransmit_sources[_s] = RetransmitState(table->igmp->get_robustness(), allow);
			} else {
				it->second.transmissions = table->igmp->get_robustness();
				it->second.allow = allow;
			}
		}
		retransmit_timer.schedule_now();
	}
	table->local().groupstate_changed(*this);
}
