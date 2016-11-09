#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "MulticastTable.hh"

CLICK_DECLS

bool MulticastTable::get(int interface) {
	auto it = table.find(interface);
	if (it == table.end()) {
		return default_value;
	} else {
		return it->second;
	}
}

void MulticastTable::set(int interface, bool val) {
	table[interface] = val;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(MulticastTable)
