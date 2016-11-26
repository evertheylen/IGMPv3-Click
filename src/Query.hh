
#pragma once

#include <cstdint>

#include <click/packet.hh>

#include "constants.hh"
#include "util.hh"

struct Query {
	Query(): reserved(0), S(false), QRV(defaults::ROBUSTNESS) {}
	
	MessageType type = MessageType::QUERY;
	MiniFloat max_resp_code = defaults::MAX_RESP_CODE;
	uint16_t checksum = 0;
	IPAddress group_address;
	uint8_t reserved : 4;
	bool S : 1;
	uint8_t QRV : 3;
	MiniFloat QQIC = defaults::QUERY_INTERVAL_CODE;
	uint8_t N = 0;
	// TODO: source addresses go here
};


class QueryBuilder {
public:
	QueryBuilder(IPAddress group_address);
	void set_checksum();
	inline Query* query() { return (Query*) packet->data(); }
	
	WritablePacket* packet;
};
