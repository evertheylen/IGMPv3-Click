#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>

#include "MCTable.hh"
#include "IGMP.hh"
#include "RouterGroupState.hh"
#include "ClientGroupState.tcc"

CLICK_DECLS

void RouterSubTable::groupstate_changed(RouterGroupState& gs) {
	// This is the lion's share of the support for routers as members
	if (parent_table->igmp->parent >= 0) {
		ClientGroupState& client = parent_table->local().get(gs.group);
		
		bool new_include = INCLUDE;
		std::set<IPAddress> new_sources;
		for (auto& it: table) {
			if (it.second.include == EXCLUDE) {
				new_include = EXCLUDE;
				break;
			}
		}
		
		// all groupstates are include
		for (auto& it: table)
			for (auto& s_it: it.second.sources)
				new_sources.insert(s_it.first);
			
		if (new_include == EXCLUDE) {
			for (auto& it: table)
				if (it.second.include)
					for (auto& s_it: it.second.sources)
						new_sources.erase(s_it.first);
			
			for (auto iter = new_sources.begin(); iter != new_sources.end();) {
				bool contained_by_all = true;
				for (auto& it: table) {
					if (it.second.sources.find(*iter) == it.second.sources.end()) {
						contained_by_all = false;
						break;
					}
				}
				if (not contained_by_all) {
					iter = new_sources.erase(iter);
				} else ++iter;
			}
		}
		
		client.change_to(new_include, new_sources);
	}
	
	SubTable<RouterGroupState>::groupstate_changed(gs);
}

CLICK_ENDDECLS

EXPORT_ELEMENT(MCTable)
