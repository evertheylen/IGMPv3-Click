
#pragma once

#include <vector>
#include <cstdint>

#include <click/packet.hh>

#include "constants.hh"
#include "util.hh"

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
	
	inline int size() { return sizeof(GroupRecord) + (sizeof(IPAddress) * ntoh_16(N)) + aux_data_len; }
	inline bool include() {
		return (type == RecordType::CHANGE_TO_INCLUDE_MODE) or (type == RecordType::MODE_IS_INCLUDE);
	}
	
	inline MemoryIterator<IPAddress> sources() {
		return MemoryIterator<IPAddress>((IPAddress*) pointer_add(this, sizeof(GroupRecord)), ntoh_16(N));
	}
	
	inline std::string description() {
		return std::string(RecordType_to_string(type)) + "(" + list_ips(sources()) + ")";
	}
};

class ReportBuilder {
public:
	ReportBuilder(uint16_t number_group_records, int tailroom = -1);
	
	template <typename Iterable>
	GroupRecord* add_record(RecordType type, IPAddress multicast_address, Iterable source_list) {
		GroupRecord* r = add_record(type, multicast_address, (int) source_list.size());
		IPAddress* write_here = (IPAddress*) (((uint8_t*) r) + sizeof(GroupRecord));
		for (IPAddress a: source_list) {
			*write_here = a;
			write_here++;
		}
		return r;
	}
	
	GroupRecord* add_record(RecordType type, IPAddress multicast_address, int sources = 0);
	
	void prepare();
	
	inline Report* report() { return (Report*) packet->data(); }
	
	WritablePacket* packet;
};

// void __template_test_ReportBuilder() {
// 	ReportBuilder r(5);
// 	std::vector<IPAddress> sources = {GENERAL_QUERY_ADDRESS, REPORT_ADDRESS};
// 	r.add_record(CHANGE_TO_(INCLUDE), GENERAL_QUERY_ADDRESS, sources);
// }
