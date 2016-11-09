
#pragma once

#include <click/config.h>
#include <clicknet/ip.h>
#include <clicknet/udp.h>
#include <clicknet/ether.h>

class MiniFloat {
public:
	uint8_t value = 0;
};


const int default_headroom = sizeof(click_ether) + sizeof(click_ip);

template <typename T>
void create_packet(T*& content, WritablePacket*& p, int packetsize = sizeof(T), int headroom = default_headroom, int tailroom = 0) {
	p = Packet::make(headroom, 0, packetsize, tailroom);
	if (p == nullptr) {
		click_chatter("Can't make packet\n");
		return;
	}
	
	content = new (p->data()) T;
}

// slightly easier, would have been a lot easier if C++ had multiple return values
#define simple_packet(type, name, packet_name) \
WritablePacket* packet_name;\
type * name;\
create_packet<type>(name, packet_name);

