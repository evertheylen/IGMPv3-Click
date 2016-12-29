
#pragma once

#include <map>
#include <memory>

#include <click/ipaddress.hh>
#include <click/timer.hh>

#include "Report.hh"
#include "Query.hh"

class MCTable;

class GroupState {
public:
	bool include = INCLUDE;
	
	MCTable* table;
	IPAddress group;
	
	GroupState(MCTable* _table = nullptr, IPAddress _group = 0);
	
	GroupState(const GroupState& other) = default;
	GroupState& operator=(const GroupState&) = default;
};


