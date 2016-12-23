
#pragma once

#include <cstdint>
#include <vector>

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
	uint16_t N = 0;
	// source addresses go here
	
	inline unsigned int size() { return sizeof(Query) + sizeof(IPAddress) * N; }
	
	inline MemoryIterator<IPAddress> sources() {
		return MemoryIterator<IPAddress>((IPAddress*) pointer_add(this, sizeof(Query)), N);
	}
};


class QueryBuilder {
public:
	template <typename Iterable>
	QueryBuilder(IPAddress group_address, Iterable source_list):
			QueryBuilder(group_address, int(source_list.size() * sizeof(IPAddress))) {
		query()->N = hton_16((uint16_t) source_list.size());
		IPAddress* write_here = (IPAddress*) (((uint8_t*) query()) + sizeof(Query));
		for (IPAddress a: source_list) {
			*write_here = a;
			write_here++;
		}
	}
	
	QueryBuilder(IPAddress group_address, int extra_room = 0);
	void prepare();
	inline Query* query() { return (Query*) packet->data(); }
	
	WritablePacket* packet;
};

// void __template_test_QueryBuilder() {
// 	std::vector<IPAddress> sources = {GENERAL_QUERY_ADDRESS};
// 	QueryBuilder qb(GENERAL_QUERY_ADDRESS, sources);
// }
