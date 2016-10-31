
#pragma once

#include <click/packet.hh>

#include "constants.hh"

struct Report {
	MessageType type = MessageType::REPORT;
	bool turn_online;
};
