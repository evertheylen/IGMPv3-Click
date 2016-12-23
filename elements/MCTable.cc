#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>

#include "MCTable.hh"
#include "IGMP.hh"

CLICK_DECLS

// nothing?

CLICK_ENDDECLS

EXPORT_ELEMENT(ClientMCTable)
EXPORT_ELEMENT(RouterMCTable)

#include "GroupState.cc"
