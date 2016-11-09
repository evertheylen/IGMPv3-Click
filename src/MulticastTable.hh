
#pragma once

#include <click/element.hh>
// fuck you click (no HashTable<int, bool>)
// #include <click/hashtable.hh>
#include <map>

class Multicast;
class IGMP;

CLICK_DECLS

class MulticastTable: public Element {
public:
	const char *class_name() const	{ return "MulticastTable"; }
	
	friend class Multicast;
	friend class IGMP;
	
	static const bool default_value = false;
	
	bool get(int interface);
	void set(int interface, bool val);
	
private:
	// For now, simple implementation: pass or no pass
	// 0 is the local interface
	std::map<int, bool> table;
};

CLICK_ENDDECLS
