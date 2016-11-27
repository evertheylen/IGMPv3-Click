#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>

#include "MulticastTable.hh"
#include "IGMP.hh"

CLICK_DECLS

// GroupState
// ==========


GroupState::GroupState(MulticastTable* _table, int _interface, IPAddress _group): 
		table(_table), interface(_interface), group(_group) {
	init_timer();
}

GroupState::GroupState(const GroupState& other): 
		include(other.include), sources(other.sources), 
		table(other.table), interface(other.interface), group(other.group) {
	init_timer();
}

GroupState& GroupState::operator=(const GroupState& other) {
	include = other.include;
	sources = other.sources;
	table = other.table;
	interface = other.interface;
	group = other.group;
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

bool GroupState::is_default() {
	return (include == DEFAULT.include)
		and (sources.empty());
		// TODO timer?
}


void GroupState::run_timer(Timer* t, void* user_data) {
	((GroupState*) user_data)->timer_expired();
}

void GroupState::timer_expired() {
	table->group_timer_expired(*this);
}

const GroupState GroupState::DEFAULT = GroupState();


// Table methods
// =============

bool MulticastTable::get(int interface, IPAddress group) {
	auto it = table.find(interface);
	if (it == table.end()) {
		return GroupState::DEFAULT.include;
	} else {
		auto it2 = it->second.find(group);
		if (it2 == it->second.end()) {
			return GroupState::DEFAULT.include;
		} else {
			return it2->second.include;
		}
	}
}

MulticastTable::SubTable& MulticastTable::get_subtable(int interface) {
	auto it = table.find(interface);
	if (it == table.end()) {
		table[interface] = SubTable();
		return table.find(interface)->second;
	}
	return it->second;
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
		gs = &(subtable[group] = GroupState(this, interface, group));
	} else {
		gs = &it2->second;
	}
	
	gs->change_to(include, interface == 0);
	if (gs->is_default()) {
		subtable.erase(group);
	}
}

void MulticastTable::group_timer_expired(GroupState& gs) {
	if (not gs.include) {
		click_chatter("GroupState timer expired, switching to INCLUDE\n");
		gs.include = INCLUDE;
		if (gs.is_default()) {
			auto it = table.find(gs.interface);
			if (it != table.end()) it->second.erase(gs.group);
		}
	}
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


CLICK_ENDDECLS
EXPORT_ELEMENT(MulticastTable)
