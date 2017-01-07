
#include "IGMP.hh"
#include "Query.hh"
#include "Report.hh"

void IGMP::push(int port, Packet* p) {
	const MessageType type = (MessageType) *p->data();
	if (type == MessageType::QUERY) {
		if (not checksum_ok<Query>(p)) return;
		Query* q = (Query*) p->data();
		parent = port;
		robustness = q->QRV;
		max_resp_time = q->max_resp_code; // TODO MiniFloat
		query_interval = q->QQIC;
		got_query(port, q, p);
	} else if (type == MessageType::REPORT) {
		if (not checksum_ok<Report>(p)) return;
		got_report(port, (Report*) p->data(), p);
	} else {
		click_chatter("%s: \tIGMP got a message with an unknown type\n", name().c_str());
	}
	p->kill();
}

// used to be in ClientIGMP
void IGMP::got_query(int port, Query* query, Packet* p) {
	// 5.2.1 + 5.2.2
	unsigned int delay = random_ms();
	uint16_t N = ntoh_16(query->N);
	if (respond_to_gq_timer.scheduled() and left(respond_to_gq_timer) < delay) {
		click_chatter("%s: \tgot (General) Query, case 5.2.1 (do nothing)\n", name().c_str());
	} else if (query->group_address == IPAddress(0)) {
		click_chatter("%s: \tgot (General) Query, case 5.2.2 (schedule GQ resp)\n", name().c_str());
		respond_to_gq_timer.schedule_after_msec(delay);
	} else {
		ClientSubTable& st = table->local();
		auto it = st.table.find(query->group_address);
		if (it == st.table.end()) return;
		ClientGroupState& gs = it->second;
		
		if (not gs.current_state_timer.scheduled()) { // 5.2.3
			click_chatter("%s: \tgot Query, case 5.2.3\n", name().c_str());
			if (N != 0) {
				click_chatter("%s: \tgot Query, case 5.2.3 !!! N = %d\n", name().c_str(), N);
				auto _sources = query->sources();
				gs.sources_to_report.insert(_sources.begin(), _sources.end());
			}
			gs.current_state_timer.schedule_after_msec(delay);
		} else {
			if (N == 0 or gs.sources_to_report.empty()) { // 5.2.4
				click_chatter("%s: \tgot Query, case 5.2.4\n", name().c_str());
				gs.sources_to_report.clear();
			} else { // 5.2.5
				click_chatter("%s: \tgot Query, case 5.2.5\n", name().c_str());
				auto _sources = query->sources();
				gs.sources_to_report.insert(_sources.begin(), _sources.end());
			}
			if (delay < left(gs.current_state_timer))
				gs.current_state_timer.schedule_after_msec(delay);
		}
	}
}

void IGMP::got_report(int port, Report* report, Packet* p) {
	click_chatter("%s: \tgot unexpected report\n", name().c_str());
}

int IGMP::configure(Vector<String>& conf, ErrorHandler* errh) {
	if (Args(conf, this, errh)
		.read_mp("TABLE", ElementCastArg("MCTable"), table)
		.consume() < 0)
		return -1;
	table->set_igmp(this);
	return 0;
}

int IGMP::initialize(ErrorHandler *errh) {
	respond_to_gq_timer.initialize(this, true);
	respond_to_gq_timer.assign(IGMP::run_respond_to_gq_timer, this);
	return 0;
}

void IGMP::run_respond_to_gq_timer(Timer* timer, void* user_data) {
	// p23, below
	IGMP* igmp = (IGMP*) user_data;
	ClientSubTable& subtable = igmp->table->local();
	int size = subtable.table.size();
	if (size > 0) {
		ReportBuilder rb;
		for (auto& gs_it: subtable.table) {
			rb.add_record(MODE_IS_(gs_it.second.include), gs_it.first, gs_it.second.sources);
		}
		rb.prepare();
		igmp->output(igmp->parent).push(rb.new_packet());
	}
}

unsigned int IGMP::random_ms() {
	std::uniform_int_distribution<unsigned int> dist(0, max_resp_time*100);
	return dist(rd);
}
