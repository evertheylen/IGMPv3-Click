
#pragma once

#include <cstdint>

#include <click/packet.hh>

#include "constants.hh"
#include "util.hh"

// TODO values for QRV and QQIC
struct Query {
	Query(): reserved(0), S(false) {}
	
	MessageType type = MessageType::QUERY;
	MiniFloat max_resp_code;
	uint16_t checksum = 0;
	IPAddress group_address;
	uint8_t reserved : 4;
	bool S : 1;
	uint8_t QRV : 3;
	uint8_t QQIC;
	uint8_t N = 0;
	// TODO: source addresses go here
};
