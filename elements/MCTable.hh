
#pragma once

#include <map>
#include <unordered_map>
#include <set>

#include <click/element.hh>
#include <click/timer.hh>

#include "constants.hh"
#include "ClientGroupState.hh"
#include "RouterGroupState.hh"
#include "IGMP.hh"

CLICK_DECLS

class IGMP;
class MCTable;

template <typename _GroupState>
class SubTable {
public:
	using GroupState = _GroupState;
	
	SubTable(MCTable* _parent = nullptr): parent_table(_parent) {}
	
	const GroupState& cget(IPAddress group) {
		auto it = table.find(group);
		if (it == table.end()) return GroupState::DEFAULT;
		else return it->second;
	}
	
	GroupState& get(IPAddress group) {
		// Creates an actual new (writable) groupstate if necessary
		auto it = table.find(group);
		if (it == table.end()) { return (table[group] = new_group_state(group)); }
		else return it->second;
	}
	
	void groupstate_changed(GroupState& gs) {
		// delete group record if it is default
		if (gs.is_default()) {
			table.erase(gs.group);
		}
	}
	
	bool empty() { return table.empty(); }
	
	std::unordered_map<IPAddress, GroupState> table;
	class MCTable* parent_table;
	
protected:
	virtual GroupState new_group_state(IPAddress group) = 0;
};


class RouterSubTable: public SubTable<RouterGroupState> {
public: 
	int interface;
	
	inline RouterSubTable(MCTable* parent = nullptr, int _interface = -1):
		SubTable<RouterGroupState>(parent), interface(_interface) {}
		
	inline RouterGroupState new_group_state(IPAddress group) {
		return RouterGroupState(parent_table, interface, group);
	}
	
	void groupstate_changed(RouterGroupState& gs);
};


class ClientSubTable: public SubTable<ClientGroupState> {
public:
	inline ClientGroupState new_group_state(IPAddress group) {
		return ClientGroupState(parent_table, group);
	}
};


class MCTable: public Element {
public:
	
	const char *class_name() const	{ return "MCTable"; }
	
	void add_handlers() {
		add_read_handler("table", table_handler, 0);
	}
	
	int initialize(ErrorHandler* e) {
		local_table.parent_table = this;
		return 0;
	}
	
	RouterSubTable& router(int interface) {
		auto it = router_tables.find(interface);
		if (it == router_tables.end()) {
			router_tables[interface] = RouterSubTable(this, interface);
			return router_tables.find(interface)->second;
		}
		return it->second;
	}
	
	ClientSubTable& local() {
		return local_table;
	}
	
	void set_igmp(IGMP* _igmp) {
		igmp = _igmp;
	}
	
	IGMP* igmp;
	
// protected:
	std::map<int, RouterSubTable> router_tables; // could be empty
	ClientSubTable local_table;
	
	
	static String table_handler(Element* e, void*) {
		return ((MCTable*) e)->print_table();
	}
	
	String print_table() {
		String s;
		if (not local_table.empty()) {
			s += String("--- Local/Total Interface ---\n");
			for (auto& it: local_table.table) {
				s += IPAddress(it.first).unparse() + "\t" + String(it.second.description().c_str()) + "\n";
			}
			s += "\n";
		}
		
		for (auto iit: router_tables) {
			s += String("--- Interface ") + String(iit.first) + " ---\n";
			for (auto& it: iit.second.table) {
				s += IPAddress(it.first).unparse() + "\t" + String(it.second.description().c_str()) + "\n";
			}
			s += "\n";
		}
		return s;
	}
};

CLICK_ENDDECLS
