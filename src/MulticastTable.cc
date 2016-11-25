#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "MulticastTable.hh"

CLICK_DECLS

// Table methods
// =============

bool MulticastTable::get(int interface, IPAddress group) {
	auto it = table.find(interface);
	if (it == table.end()) {
		return default_value;
	} else {
		auto it2 = it->second.find(group);
		if (it2 == it->second.end()) {
			return default_value;
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
	auto it2 = subtable.find(group);
	if (it2 == subtable.end()) {
		// emplace true
		click_chatter("subemplacing %d, %d\n", group, include);
		subtable.emplace(group, include);
	} else {
		it2->second.include = include;
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

CLICK_ENDDECLS
EXPORT_ELEMENT(MulticastTable)
