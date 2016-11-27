
#pragma once

#include <map>
#include <unordered_map>
#include <set>

#include <click/element.hh>
#include <click/timer.hh>

#include "constants.hh"

CLICK_DECLS

class MulticastTable;

class GroupState {
public:
	// state:
	bool include = INCLUDE;
	std::set<IPAddress> sources; // = {}
	Timer timer;
	
	// other direction
	MulticastTable* table;
	int interface;
	IPAddress group;
	
	GroupState(MulticastTable* _table = nullptr, int _interface = -1, IPAddress _group = 0);
	GroupState(const GroupState& other);
	GroupState& operator=(const GroupState&);
	void init_timer();
	
	// set and process timers etc
	void change_to(bool _include, bool local);
	
	bool is_default();
	
	// fired on timer expiry
	static void run_timer(Timer* t, void* user_data);
	void timer_expired();
	
	static const GroupState DEFAULT;
};

namespace std {
	template <> struct hash<IPAddress> {
		size_t operator()(const IPAddress& ip) const {
			return ip.hashcode();
		}
	};
}

class IGMP;

// IP Addresses are saved as much as possible in network order!
// The MulticastTable won't do any effort converting them.
class MulticastTable: public Element {
public:
	using SubTable = std::unordered_map<IPAddress, GroupState>;
	
	const char *class_name() const	{ return "MulticastTable"; }
	void add_handlers();
	
	bool get(int interface, IPAddress group);
	void set(int interface, IPAddress group, bool include);
	SubTable& get_subtable(int interface);
	
	void group_timer_expired(GroupState& gs);
	
	void set_igmp(IGMP* igmp);
	
	friend class GroupState;
	
private:
	// 0 is the local interface
	std::map<int, SubTable> table;
	
	IGMP* igmp;
	
	static String table_handler(Element* e, void*);
	String print_table();
};

CLICK_ENDDECLS
