
#include <clicknet/ip.h>

#include "Query.hh"

QueryBuilder::QueryBuilder(IPAddress group_address) {
	packet = Packet::make(default_headroom, nullptr, sizeof(Query), 0);
	Query* query = new (packet->data()) Query;
	query->group_address = group_address;
}

void QueryBuilder::prepare() {
	if (query()->group_address == IPAddress(0)) { // general query
		packet->set_dst_ip_anno(GENERAL_QUERY_ADDRESS);
	} else { // group-specific query
		packet->set_dst_ip_anno(query()->group_address);
	}
	query()->checksum = 0;
	query()->checksum = click_in_cksum(packet->data(), packet->length());
}

