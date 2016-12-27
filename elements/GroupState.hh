
#pragma once

#include <map>
#include <memory>

#include <click/ipaddress.hh>
#include <click/timer.hh>

#include "Report.hh"
#include "Query.hh"

template <typename Table>
class GroupState {
public:
	bool include = INCLUDE;
	
	Table* table;
	int interface;
	IPAddress group;
	
	GroupState(void* _table = nullptr, int _interface = -1, IPAddress _group = 0);
	
	GroupState(const GroupState& other) = default;
	GroupState& operator=(const GroupState&) = default;
};


