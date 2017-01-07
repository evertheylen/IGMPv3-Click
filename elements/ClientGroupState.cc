
#include "ClientGroupState.hh"
#include "MCTable.hh"
#include "IGMP.hh"

ClientGroupState::ClientGroupState(MCTable* _table, IPAddress group): 
		GroupState(_table, group) {
	init_timers();
}

ClientGroupState::ClientGroupState(const ClientGroupState& other):
		GroupState(other), retransmit_mode(other.retransmit_mode),
		retransmit_sources(other.retransmit_sources) {}

ClientGroupState& ClientGroupState::operator=(const ClientGroupState& other) {
	GroupState::operator=(other);
	sources = other.sources;
	retransmit_sources = other.retransmit_sources;
	retransmit_mode = other.retransmit_mode;
	init_timers();
	return *this;
}

void ClientGroupState::init_timers() {
	if (table != nullptr) {
		retransmit_timer.initialize(table, true);
		retransmit_timer.assign(ClientGroupState::run_retransmit_timer, this);
		current_state_timer.initialize(table, true);
		current_state_timer.assign(ClientGroupState::run_current_state_timer, this);
	}
}

bool ClientGroupState::forward(IPAddress source) const {
	if (sources.find(source) != sources.end()) return include;
	else return not include;
}

bool ClientGroupState::is_default() const {
	return (include == DEFAULT.include) and sources.empty()
		and retransmit_sources.empty() and retransmit_mode == 0
		and (not current_state_timer.scheduled());
}

std::string ClientGroupState::description() {
	return std::string(Include_to_string(include)) + "(" + list_ips(sources) + ")";
}


// Changes and Retransmitting
// --------------------------

GroupRecord* ClientGroupState::add_filter_mode_record(ReportBuilder& rb) {
	GroupRecord* r = rb.add_record(CHANGE_TO_(include), group, sources);
	retransmit_mode = retransmit_mode==0 ? 0 : retransmit_mode-1;
	for (IPAddress s: sources) {
		auto it = retransmit_sources.find(s);
		if (it != retransmit_sources.end()) {
			it->second.transmissions--;
			if (it->second.transmissions == 0) it = retransmit_sources.erase(it);
		}
	}
	return r;
}

void ClientGroupState::retransmit_timer_expired() {
	ReportBuilder rb;
	
	#define print(x) click_chatter("%s: \tSending to router: %s", table->name().c_str(), x->description().c_str())
	
	if (retransmit_mode > 0) {
		print(add_filter_mode_record(rb));
	}
	
	std::vector<IPAddress> allow_sources, block_sources;
	for (auto it = retransmit_sources.begin(); it != retransmit_sources.end(); ) {
		if (it->second.allow) allow_sources.push_back(it->first);
		else block_sources.push_back(it->first);
		
		it->second.transmissions--;
		if (it->second.transmissions == 0)
			it = retransmit_sources.erase(it);
		else ++it;
	}
	
	if (not allow_sources.empty())
		print(rb.add_record(RecordType::ALLOW_NEW_SOURCES, group, allow_sources));
	if (not block_sources.empty())
		print(rb.add_record(RecordType::BLOCK_OLD_SOURCES, group, block_sources));
	
	if (rb.records > 0) {
		rb.prepare();
		Packet* p = rb.new_packet();
		table->igmp->output(table->igmp->parent).push(p);
	}
	
	if (retransmit_mode > 0 or (not retransmit_sources.empty()))
		retransmit_timer.reschedule_after_msec(random_ms());
	
	table->local().groupstate_changed(*this);
}

void ClientGroupState::run_retransmit_timer(Timer* t, void* user_data) {
	((ClientGroupState*) user_data)->retransmit_timer_expired();
}

unsigned int ClientGroupState::random_ms() {
	std::uniform_int_distribution<unsigned int> dist(0, table->igmp->URI()*1000);
	return dist(rd);
}

#include "ClientGroupState.tcc"


// Reporting Current State
// -----------------------

void ClientGroupState::run_current_state_timer(Timer* t, void* user_data) {
	((ClientGroupState*) user_data)->current_state_timer_expired();
}

void ClientGroupState::current_state_timer_expired() {
	click_chatter("%s: \tCurrent state timer expired for ClientGroupState = %s\n", 
				  table->name().c_str(), description().c_str());
	if (is_default()) {
		table->local().groupstate_changed(*this);
		return;
	}
	
	ReportBuilder rb;
	if (sources_to_report.empty()) {
		// p24, point 2
		rb.add_record(MODE_IS_(include), group, sources);
	} else {
		// p24, point 3
		auto res = include ? sources&sources_to_report : sources_to_report-sources;
		if (not res.empty()) rb.add_record(MODE_IS_(include), group, res);
		sources_to_report.clear();
	}
	
	if (rb.records > 0) {
		rb.prepare();
		table->igmp->output(table->igmp->parent).push(rb.new_packet());
	}
}

const ClientGroupState ClientGroupState::DEFAULT = ClientGroupState();
