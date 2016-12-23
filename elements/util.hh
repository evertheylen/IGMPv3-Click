#pragma once

#include <click/config.h>
#include <clicknet/ip.h>
#include <clicknet/udp.h>
#include <clicknet/ether.h>

#include <stdexcept>
#include <cstdint>
#include <cmath>
#include <vector>
#include <set>

using MiniFloat = uint8_t; // TODO

// TODO sort this out...
// It seems it is impossible to maintain a sum while running, you have to recalculate
// it every time (except for some specific cases). The reason for this is being
// able to switch from EXCLUDE{} to some INCLUDE state. (--> see timers in GroupState!)
class SourceState {
public:
	SourceState(bool _include = true, const std::set<IPAddress>& sources = {});
	
	template <typename Iterable>
	void add(bool _include, const Iterable& _sources) {
		if (include) {
			if (_include) {
				sources.insert(_sources.begin(), _sources.end());
			} else {
				include = false;
				std::set<IPAddress> new_sources;
				for (IPAddress s: _sources)
					if (sources.find(s) == sources.end())
						new_sources.insert(s);
				
				sources = new_sources;
			}
		} else {
			if (_include) {
				sources.erase(_sources.begin(), _sources.end());
			} else {
				
				//the_set.insert(source);
			}
		}
	}

	bool include;
	std::set<IPAddress> sources;
};

// http://cpp.sh/6zfcn
template <typename T>
class MemoryIterator {
	T* ptr;
	unsigned int N;
public:
	MemoryIterator(T* _ptr, unsigned int _N): ptr(_ptr), N(_N) {}
	inline T* begin() { return ptr; }
	inline T* end() { return ptr + N; }
	inline bool contains(const T& val) {
		for (T it: *this) {
			if (it == val) return true;
		}
		return false;
	}
};

using centiseconds = unsigned int;
using seconds = unsigned int;
using milliseconds = unsigned int;

#define def(name, type, func)\
inline type name (type input) { return func(input); }\
template<typename T> type name (T input) = delete;

def(ntoh_16, uint16_t, ntohs)
def(ntoh_32, uint32_t, ntohl)
def(ntoh_64, uint64_t, ntohq)

def(hton_16, uint16_t, htons)
def(hton_32, uint32_t, htonl)
def(hton_64, uint64_t, htonq)

template <typename T>
T* pointer_add(T* ptr, int n) {
	char* _ptr = (char*) ptr;
	_ptr += n;
	return (T*) _ptr;
}

template <typename T>
bool checksum_ok(Packet* p) {
	T* obj = (T*) p->data();
	uint16_t checksum = obj->checksum;
	obj->checksum = 0;
	bool ok = (click_in_cksum(p->data(), p->length()) == checksum);
	obj->checksum = checksum;
	if (not ok) click_chatter("Checksum not ok!");
	return ok;
}

template <typename T>
std::string list_ips(T iterable) {
	std::string s;
	for (auto it: iterable) s += std::string(it.unparse().c_str()) + ", ";
	if (s.length() > 0) s = s.substr(0, s.length()-2);
	return s;
}

template <typename T>
std::string list_first_ips(T iterable) {
	std::string s;
	for (auto it: iterable) s += std::string(it.first.unparse().c_str()) + ", ";
	if (s.length() > 0) s = s.substr(0, s.length()-2);
	return s;
}

namespace std {
	template <> struct hash<IPAddress> {
		size_t operator()(const IPAddress& ip) const {
			return ip.hashcode();
		}
	};
}

inline bool operator<(const IPAddress& a, const IPAddress& b) {
	return ntoh_32((uint32_t) a) < ntoh_32((uint32_t) b);
}

std::vector<String> split(const String& s, char delim);
