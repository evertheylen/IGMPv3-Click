#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>

#include "MulticastTable.hh"
#include "IGMP.hh"

CLICK_DECLS

// GroupState
// ==========


GroupState::GroupState(MulticastTable* _table): table(_table) { 
	init_timer();
}

GroupState::GroupState(const GroupState& other): include(other.include), sources(other.sources), table(other.table) {
	init_timer();
}

GroupState& GroupState::operator=(const GroupState& other) {
	include = other.include;
	sources = other.sources;
	table = other.table;
	init_timer();
	return *this;
}

void GroupState::init_timer() {
	if (table != nullptr) {
		timer.initialize(table);
		timer.assign(GroupState::run_timer, this);
	}
}

void GroupState::change_to(bool _include, bool local) {
	if (not local) {
		// literal translation of table in section 6.4.1 (minus sources)
		if (include) {
			if (_include) {
				// (B) = GMI
			} else {
				// (B-A) = 0
				// Delete (A-B)
				// Group Timer = GMI
				timer.schedule_after_msec(table->igmp->GMI() * 100);
			}
		} else {
			if (_include) {
				// (A) = GMI
			} else {
				// (A-X-Y) = GMI
				// Delete (X-A)
				// Delete (Y-A)
				// Group Timer = GMI
				timer.schedule_after_msec(table->igmp->GMI() * 100);
			}
		}
	}
	
	include = _include;
}

void GroupState::run_timer(Timer* t, void* user_data) {
	((GroupState*) user_data)->timer_expired();
}

void GroupState::timer_expired() {
	click_chatter("GroupState timer expired\n");
}


// Table methods
// =============

bool MulticastTable::get(int interface, IPAddress group) {
	auto it = table.find(interface);
	if (it == table.end()) {
		return default_group_state.include;
	} else {
		auto it2 = it->second.find(group);
		if (it2 == it->second.end()) {
			return default_group_state.include;
		} else {
			return it2->second.include;
		}
	}
}

void MulticastTable::set(int interface, IPAddress group, bool include) {
	auto it = table.find(interface);
	
	if (it == table.end()) {
		// 1 bucket minimum
		//it = table.emplace(interface, 1).first;  // works in clang...
		table[interface] = SubTable();
		it = table.find(interface);
		click_chatter("emplacing %d\n", interface);
	}
	
	auto& subtable = it->second;
	GroupState* gs;
	auto it2 = subtable.find(group);
	if (it2 == subtable.end()) {
		// emplace true
		click_chatter("subemplacing %d, %d\n", group, include);
		gs = &(subtable[group] = GroupState(this));
	} else {
		gs = &it2->second;
	}
	
	gs->change_to(include, interface == 0);
}


// Handler stuff
// =============

void MulticastTable::add_handlers() {
	add_read_handler("table", table_handler, 0);
}

String MulticastTable::table_handler(Element* e, void*) {
	return ((MulticastTable*) e)->print_table();
}

String MulticastTable::print_table() {
	String s;
	for (auto iit: table) {
		s += String("--- Interface ") + String(iit.first) + " ---\n";
		for (auto it: iit.second) {
			s += IPAddress(it.first).unparse() + "\t" + String((uint32_t) it.first) + "\t" + (it.second.include? "INCLUDE" : "EXCLUDE") + "\n";
		}
		s += "\n";
	}
	return s;
}


// Etc
// ===

void MulticastTable::set_igmp(IGMP* _igmp) {
	igmp = _igmp;
}

const GroupState MulticastTable::default_group_state = GroupState();

CLICK_ENDDECLS
EXPORT_ELEMENT(MulticastTable)
