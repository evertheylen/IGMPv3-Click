
#pragma once

#include <click/element.hh>
// fuck you click (no HashTable<int, bool>)
// #include <click/hashtable.hh>
#include <map>
#include <unordered_map>
#include <set>

#include "constants.hh"

CLICK_DECLS

struct GroupState {
	bool include;
	std::set<IPAddress> sources;
	
	GroupState(bool include = true): include(include) {}
};

namespace std {
	template <> struct hash<IPAddress> {
		size_t operator()(const IPAddress& ip) const {
			return ip.hashcode();
		}
	};
}

// IP Addresses are saved as much as possible in network order!
// The MulticastTable won't do any effort converting them.
class MulticastTable: public Element {
public:
	const char *class_name() const	{ return "MulticastTable"; }
	void add_handlers();
	
	static const bool default_value = INCLUDE;
	
	bool get(int interface, IPAddress group);
	void set(int interface, IPAddress group, bool include);
	
private:
	// 0 is the local interface
	using SubTable = std::unordered_map<IPAddress, GroupState>;
	std::map<int, SubTable> table;
	
	//static String table_handler(Element* , void*);
	static String table_handler(Element* e, void*);
	String print_table();
};


CLICK_ENDDECLS
