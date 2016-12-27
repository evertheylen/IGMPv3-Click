
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
		click_chatter("%s: \tGroupState = %s --> %s\n", name().c_str(), gs_it->second.description().c_str(),
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
	
	void groupstate_changed(GroupState& gs) {
		// delete group record if it is default
		if (gs.is_default()) {
			get_subtable(gs.interface).erase(gs.group);
		}
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
				s += IPAddress(it.first).unparse() + "\t" + String(it.second.description().c_str()) + "\n";
			}
			s += "\n";
		}
		return s;
	}
};

#include "ClientGroupState.hh"
#include "RouterGroupState.hh"

//using ClientMCTable = MCTable<ClientGroupState>;
//using RouterMCTable = MCTable<RouterGroupState>;

class ClientMCTable: public MCTable<ClientGroupState> {
public:
	const char *class_name() const	{ return "ClientMCTable"; }
};

class RouterMCTable: public MCTable<RouterGroupState> {
public:
	const char *class_name() const	{ return "RouterMCTable"; }
};

CLICK_ENDDECLS
