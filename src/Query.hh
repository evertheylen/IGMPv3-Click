
#pragma once

#include <click/packet.hh>

#include "constants.hh"

struct Query {
	MessageType type = MessageType::QUERY;
	bool online;
};
