
#pragma once

#include <cstdint>

#include <click/packet.hh>

#include "constants.hh"

struct Report {
	MessageType type = MessageType::REPORT;
	uint8_t reserved = 0;
	uint16_t checksum = 0;
	uint16_t reserved2 = 0;
	uint16_t number_group_records = 0;
	// group records go here
};

struct GroupRecord {
	RecordType type;
	uint8_t aux_data_len = 0;
	uint16_t N = 0;
	IPAddress multicast_address;
	// Source addresses go here
	// Auxiliary data goes here
};
