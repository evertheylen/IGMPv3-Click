
#include <algorithm>

#include "RouterGroupState.hh"
#include "MCTable.hh"
#include "IGMP.hh"

SourceTimer::SourceTimer(RouterGroupState* _gs, IPAddress _source, unsigned int milliseconds):
		gs(_gs), source(_source), timer(RouterGroupState::run_source_timer, this) {
	timer.initialize(gs->table, true);
	if (milliseconds > 0) {
		timer.schedule_after_msec(milliseconds);
	}
}

RouterGroupState::RouterGroupState(MCTable* _table, int _interface, IPAddress _group):
		GroupState(_table, _group), interface(_interface) {
	init_timers();
}

RouterGroupState::RouterGroupState(const RouterGroupState& other): 
		GroupState(other), sources(other.sources), interface(other.interface) {
	init_timers();
}

RouterGroupState& RouterGroupState::operator=(const RouterGroupState& other) {
		GroupState::operator=(other);
	interface = other.interface;
	sources = other.sources;
	init_timers();
	return *this;
}

void RouterGroupState::init_timers() {
	if (table != nullptr) {
		group_timer.initialize(table, true);
		group_timer.assign(RouterGroupState::run_group_timer, this);
	}
}

bool RouterGroupState::forward(IPAddress source) const {
	auto it = sources.find(source);
	if (it == sources.end()) return not include;
	// 6.2.3: "If a source record has a running timer with a router filter-mode for the group
	// of EXCLUDE, it means that at least one system desires the source. It should therefore
	// be forwarded on the network.
	return include ? true : it->second->timer.scheduled();
}

bool RouterGroupState::is_default() const {
	return (include == DEFAULT.include) and (sources.empty());
}

void RouterGroupState::run_group_timer(Timer* t, void* user_data) {
	((RouterGroupState*) user_data)->group_timer_expired();
}

void RouterGroupState::group_timer_expired() {
	// see page 28 (top)
	click_chatter("%s: \tRouterGroupState timer expired, switching to INCLUDE\n", table->igmp->name().c_str());
	include = INCLUDE;
	// erase all sources with non-running timers
	for (auto it = sources.cbegin(); it != sources.cend(); ) {
		if (not it->second->timer.scheduled()) {
			it = sources.erase(it);
		} else ++it;
	}
	// will delete the group record if it turns out to be DEFAULT
	table->router(interface).groupstate_changed(*this);
}

void RouterGroupState::run_source_timer(Timer* t, void* user_data) {
	SourceTimer* source_timer = (SourceTimer*) user_data;
	source_timer->gs->source_timer_expired(source_timer);
}

void RouterGroupState::source_timer_expired(SourceTimer* source_timer) {
	click_chatter("%s: \tSourceTimer expired, for source %s\n", table->igmp->name().c_str(), source_timer->source.unparse().c_str());
	if (include) {
		sources.erase(source_timer->source);
	}
	// if exclude, then do nothing (see page 29, top)
	table->router(interface).groupstate_changed(*this);
}

void RouterGroupState::schedule_source(IPAddress ip, unsigned int milliseconds) {
	if (sources.find(ip) == sources.end()) {
		sources[ip] = std::make_shared<SourceTimer>(this, ip, milliseconds);
	} else {
		if (milliseconds == 0) sources[ip]->timer.unschedule();
		else sources[ip]->timer.schedule_after_msec(milliseconds);
	}
}

void RouterGroupState::Q() { // Group-Specific Query
	QueryBuilder qb(group, 0);
	qb.query()->QRV = table->igmp->get_robustness();
	qb.prepare();
	table->igmp->output(interface).push(qb.new_packet());
	unsigned int LMQT_ms = table->igmp->LMQT() * 100;
	if (LMQT_ms < left(group_timer)) {
		group_timer.schedule_after_msec(LMQT_ms);
	}
}

template <typename Iterable>
void RouterGroupState::Q(Iterable A) {  // Group-And-Source-Specific Query
	unsigned int LMQT_ms = table->igmp->LMQT() * 100;
	for (IPAddress a: A) {
		auto a_it = sources.find(a);
		if (a_it != sources.end()) {
			unsigned int left_ms = left(a_it->second->timer);
			if (LMQT_ms < left_ms) {
				a_it->second->timer.schedule_after_msec(LMQT_ms);
			}
		}
	}
	QueryBuilder qb(group, A);
	qb.query()->QRV = table->igmp->get_robustness();
	qb.prepare();
	this->table->igmp->output(interface).push(qb.new_packet());
}

std::string RouterGroupState::description() {
	std::string res(Include_to_string(include));
	res += "(";
	if (include) {
		res += list_first_ips(sources);
	} else {
		std::vector<IPAddress> A, B;
		for (const auto& it: sources) {
			if (it.second->timer.scheduled()) A.push_back(it.first);
			else B.push_back(it.first);
		}
		res += std::string("{") + list_ips(A) + "}, {" + list_ips(B) + "}";
	}
	res += ")";
	return res;
}


void RouterGroupState::got_current_state_record(GroupRecord* record) {
	unsigned int GMI_ms = 100 * table->igmp->GMI();
	
	// See 6.4.1
	// Notation: (A,B) represents all the sources, where A has source timers > 0 (redundant included),
	// and B has source timers == 0 (actually excluded). B is only of importance in EXCLUDE mode.
	// ### [Router State]    [Report Rec'd]    [New Router State]
	
	if (record->type == RecordType::MODE_IS_INCLUDE) {
		// ### INCLUDE (A)     IS_IN (B)   INCLUDE (A+B)
		// (B) = GMI
		// ### EXCLUDE (X, Y)  IS_IN (B)   EXCLUDE (X+B, Y-B)     (A ~~> B)
		// (B) = GMI
		for (IPAddress b: record->sources()) schedule_source(b, GMI_ms);
	} else {
		if (include) {
			// ### INCLUDE (A)    IS_EX (B)    EXCLUDE (A*B, B-A)
			// (B-A) = 0
			// Delete (A-B)
			// Group Timer = GMI
			
			include = EXCLUDE;
			
			for (auto it = sources.cbegin(); it != sources.cend(); ) {
				if (not record->sources().contains(it->first)) { // linear search
					// A-B. This deletes all that are in A, but not in B
					it = sources.erase(it);
				} else ++it;
			}
			// afterwards, only A*E will be left (with running timers) in sources
			
			for (IPAddress b: record->sources()) {
				// this now becomes `b in (A*B)`
				if (sources.find(b) == sources.end()) {
					// B-A ~~> B-(A*B)  (same)
					sources[b] = std::make_shared<SourceTimer>(this, b); // no timer
				}
			}
			// afterwards, we've added (B-A) to the sources, with no timer
		} else {
			// ### EXCLUDE (X, Y)    IS_EX (A)    EXCLUDE (A-Y, Y*A)
			// (A-X-Y) = GMI
			// Delete (X-A)
			// Delete (Y-A)
			// Group Timer = GMI
			
			for (IPAddress a: record->sources()) {
				if (sources.find(a) == sources.end()) {
					// (A-X-Y) = GMI
					sources[a] = std::make_shared<SourceTimer>(this, a, GMI_ms);
				}
			}
			// afterwards, we've added A to the sources with a timer (at least those that weren't 
			// already in there). The current state is then:
			// EXCLUDE (X + (A-X-Y)), Y)
			
			for (auto xy_it = sources.cbegin(); xy_it != sources.cend(); ) {
				if (not record->sources().contains(xy_it->first)) { // linear search
					// Delete (X+Y) - A
					xy_it = sources.erase(xy_it);
				} else ++xy_it;
			}
		}
		// Both cases need their group_timer set to GMI
		group_timer.schedule_after_msec(GMI_ms);
	}
}

void RouterGroupState::got_state_change_record(GroupRecord* record) {
	unsigned int GMI_ms = 100 * table->igmp->GMI();
	unsigned int GT_ms = left(group_timer);
	
	// See 6.4.2
	// ### [Router State]    [Report Rec'd]    [New Router State]
	if (record->type == RecordType::ALLOW_NEW_SOURCES) {
		// ### INCLUDE (A)      ALLOW (B)    INCLUDE(A+B)
		// (B) = GMI
		// ### EXCLUDE (X, Y)   ALLOW (B)    EXCLUDE(X+B, Y-B)
		// (B) = GMI
		for (IPAddress b: record->sources()) schedule_source(b, GMI_ms);
	} else if (record->type == RecordType::BLOCK_OLD_SOURCES) {
		if (include) {
			// ### INCLUDE (A)      BLOCK (B)      INCLUDE (A)
			// Send Q(G, A*B)
			std::vector<IPAddress> A_and_B;
			for (IPAddress b: record->sources())
				if (sources.find(b) != sources.end())
					A_and_B.push_back(b);
			
			Q(A_and_B);
		} else {
			// ### EXCLUDE (X,Y)    BLOCK (A)      EXCLUDE (X+(A-Y), Y)
			// (A-X-Y) = Group Timer
			// Send Q(G, A-Y)
			std::vector<IPAddress> A_min_Y;
			for (IPAddress a: record->sources()) {
				auto it = sources.find(a);
				if (it == sources.end()) { // not in X or Y => (A-X-Y)
					schedule_source(a, GT_ms);
					A_min_Y.push_back(a);
				} else if (it->second->timer.scheduled()) { // in X, but not Y
					A_min_Y.push_back(a);
				}
			}
			Q(A_min_Y);
		}
	} else if (record->type == RecordType::CHANGE_TO_EXCLUDE_MODE) {
		if (include) {
			// ### INCLUDE (A)      TO_EX (B)      EXCLUDE (A*B, B-A)
			// (B-A) = 0
			// Delete (A-B)
			// Send Q(G, A*B)
			// Group Timer = GMI
			include = EXCLUDE;
			
			std::vector<IPAddress> A;
			for (auto a: sources) A.push_back(a.first);
			std::set<IPAddress> A_and_B;
			for (IPAddress b: record->sources()) {
				if (sources.find(b) == sources.end()) {
					schedule_source(b, 0); // (B-A)
				} else {
					A_and_B.insert(b);
				}
			}
			
			for (IPAddress a: A) {
				if (A_and_B.find(a) == A_and_B.end()) { // A-B
					auto a_it = sources.find(a);
					sources.erase(a_it);
				}
			}
			
			Q(A_and_B);
			
		} else {
			// ### EXCLUDE (X,Y)    TO_EX (A)      EXCLUDE (A-Y, Y*A)
			// (A-X-Y) = Group Timer
			// Delete (X-A)
			// Delete (Y-A)
			// Send Q(G, A-Y)
			// Group Timer = GMI
			
			std::vector<IPAddress> A_min_Y;
			for (IPAddress a: record->sources()) {
				auto it = sources.find(a);
				if (it == sources.end()) { // not in X or Y => (A-X-Y)
					schedule_source(a, GT_ms);
					A_min_Y.push_back(a);
				} else if (it->second->timer.scheduled()) { // in X, but not Y
					A_min_Y.push_back(a);
				}
			}
			
			for (auto xy_it = sources.cbegin(); xy_it != sources.cend(); ) {
				if (not record->sources().contains(xy_it->first)) { // linear search
					// Delete (X+Y) - A
					xy_it = sources.erase(xy_it);
				} else ++xy_it;
			}
			
			Q(A_min_Y);
		}
	} else if (record->type == RecordType::CHANGE_TO_INCLUDE_MODE) {
		for (IPAddress record_ip: record->sources()) {
			schedule_source(record_ip, GMI_ms);
		}
		
		if (include) {
			// ### INCLUDE (A)      TO_IN (B)      INCLUDE (A+B)
			// (B) = GMI
			// Send Q(G, A-B)
			
			std::vector<IPAddress> A_min_B;
			for (auto a_it: sources) {
				if (not record->sources().contains(a_it.first)) {
					A_min_B.push_back(a_it.first);
				}
			}
			Q(A_min_B);
		} else {
			// ### EXCLUDE (X,Y)    TO_IN (A)      EXCLUDE (X+A, Y-A)
			// (A) = GMI
			// Send Q(G, X-A)
			// Send Q(G)
			
			std::vector<IPAddress> X_min_A;
			for (auto xy_it: sources) {
				if (xy_it.second->timer.scheduled()) { // X
					if (not record->sources().contains(xy_it.first)) {
						X_min_A.push_back(xy_it.first);
					}
				}
			}
			Q(X_min_A);
			Q();
		}
	}
}

const RouterGroupState RouterGroupState::DEFAULT = RouterGroupState();
