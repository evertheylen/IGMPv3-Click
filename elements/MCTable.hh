
#pragma once

#include <map>
#include <unordered_map>
#include <set>

#include <click/element.hh>
#include <click/timer.hh>

#include "constants.hh"
#include "GroupState.hh"

CLICK_DECLS

class IGMP;

// Used by MC
class BaseMCTable: public Element {
public:
	const char *class_name() const	{ return "_BaseMCTable"; }
	virtual bool get(int interface, IPAddress group, IPAddress source) = 0;
};

// IP Addresses are saved as much as possible in network order!
// The MCTable won't do any effort converting them.
template <typename _GroupState>
class MCTable: public BaseMCTable {
public:
	using GroupState = _GroupState;
	using SubTable = std::unordered_map<IPAddress, GroupState>;
	
	void add_handlers() {
		add_read_handler("table", table_handler, 0);
	}
	
	bool get(int interface, IPAddress group, IPAddress source) {
		auto it = table.find(interface);
		if (it == table.end()) return false;
		
		auto gs_it = it->second.find(group);
		if (gs_it == it->second.end()) return false;
		
		bool may_pass = gs_it->second.forward(source);
		click_chatter("GroupState = %s --> %s\n", gs_it->second.description().c_str(),
					  may_pass ? "Go along..." : "DENIED");
		return may_pass;
	}
	
	SubTable& get_subtable(int interface) {
		auto it = table.find(interface);
		if (it == table.end()) {
			table[interface] = SubTable();
			return table.find(interface)->second;
		}
		return it->second;
	}
	
	const GroupState& cget_groupstate(int interface, IPAddress group) {
		SubTable& sub = get_subtable(interface);
		auto it = sub.find(group);
		if (it == sub.end()) return GroupState::DEFAULT;
		else return it->second;
	}
	
	GroupState& get_groupstate(int interface, IPAddress group) {
		// Creates an actual new (writable) groupstate if necessary
		SubTable& sub = get_subtable(interface);
		auto it = sub.find(group);
		if (it == sub.end()) { return (sub[group] = GroupState(this, interface, group)); }
		else return it->second;
	}
	
	void set_igmp(IGMP* _igmp) {
		igmp = _igmp;
	}
	
	friend GroupState;
	
protected:
	std::map<int, SubTable> table;
	
	IGMP* igmp;
	
	static String table_handler(Element* e, void*) {
		return ((MCTable<GroupState>*) e)->print_table();
	}
	
	String print_table() {
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
};

class ClientMCTable: public MCTable<ClientGroupState> {
public:
	const char *class_name() const	{ return "ClientMCTable"; }
	
	using GroupState = ClientGroupState;
	using SubTable = MCTable<ClientGroupState>::SubTable;
	
	using MCTable<ClientGroupState>::MCTable;
};

class RouterMCTable: public MCTable<RouterGroupState> {
public:
	const char *class_name() const	{ return "RouterMCTable"; }
	
	using GroupState = RouterGroupState;
	using SubTable = MCTable<RouterGroupState>::SubTable;
	
	using MCTable<RouterGroupState>::MCTable;
	
	void groupstate_changed(GroupState& gs) {
		// delete group record if it is default
		if (gs.is_default()) {
			get_subtable(gs.interface).erase(gs.group);
		}
	}
};

CLICK_ENDDECLS


