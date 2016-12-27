
#pragma once

#include "GroupState.hh"

template <typename Table>
GroupState<Table>::GroupState(void* _table, int _interface, IPAddress _group):
	table((Table*) _table), interface(_interface), group(_group) {}

