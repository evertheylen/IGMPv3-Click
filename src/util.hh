#pragma once

#include <click/config.h>
#include <clicknet/ip.h>
#include <clicknet/udp.h>
#include <clicknet/ether.h>

#include <stdexcept>
#include <cstdint>
#include <cmath>

//4.1.7, p10
struct MiniFloat {
	MiniFloat() = default;
	MiniFloat(unsigned int i);
	MiniFloat(const MiniFloat& other);
	
	void set(unsigned int i);  // exception if too big or negative (or assert)
	unsigned int get();  // should always work
	unsigned int get_int();  // works only if the byte is actually an int, otherwise an exception

private:
	uint8_t byte;
};

using centiseconds = unsigned int;
using seconds = unsigned int;

const int default_headroom = sizeof(click_ether) + sizeof(click_ip);

#define def_ntoh(len, type, func)\
inline type ntoh_ ## len (type input) { return func(input); }\
template<typename T> type ntoh_ ## len (T input) = delete;

def_ntoh(16, uint16_t, ntohs)
def_ntoh(32, uint32_t, ntohl)
def_ntoh(64, uint64_t, ntohq)


#define def_hton(len, type, func)\
inline type hton_ ## len (type input) { return func(input); }\
template<typename T> type hton_ ## len (T input) = delete;

def_hton(16, uint16_t, htons)
def_hton(32, uint32_t, htonl)
def_hton(64, uint64_t, htonq)

template <typename T>
void pointer_add(T*& ptr, int n) {
	char* _ptr = (char*) ptr;
	_ptr += n;
	ptr = (T*) _ptr;
}
