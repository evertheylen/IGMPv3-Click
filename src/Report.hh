
#pragma once

#include <initializer_list>
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
	
	inline int size() { return sizeof(GroupRecord) + (sizeof(IPAddress) * N) + aux_data_len; }
	inline bool include() { 
		return (type == RecordType::CHANGE_TO_INCLUDE_MODE) or (type == RecordType::MODE_IS_INCLUDE);
	}
};

// report builder does NOT care about aux data
class ReportBuilder {
public:
	ReportBuilder(uint16_t number_group_records, int tailroom = -1);
	
	GroupRecord* add_record(RecordType type, IPAddress multicast_address, std::initializer_list<IPAddress> sources, int extra_sources = 0);
	
	inline GroupRecord* add_record(RecordType type, IPAddress multicast_address, int extra_sources = 0) {
		return add_record(type, multicast_address, {}, extra_sources);
	}
	
	void set_checksum();
	
	inline Report* report() { return (Report*) packet->data(); }
	
	WritablePacket* packet;
};

