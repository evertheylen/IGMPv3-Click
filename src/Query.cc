
#include <clicknet/ip.h>

#include "Query.hh"

QueryBuilder::QueryBuilder(IPAddress group_address) {
	packet = Packet::make(default_headroom, nullptr, sizeof(Query), 0);
	Query* query = new (packet->data()) Query;
	query->group_address = group_address;
}

void QueryBuilder::set_checksum() {
	query()->checksum = 0;
	query()->checksum = click_in_cksum(packet->data(), packet->length());
}

