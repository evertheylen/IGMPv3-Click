
#pragma once

#include <cstdint>

#include <click/config.h>
#include <clicknet/ip.h>
#include <clicknet/udp.h>
#include <clicknet/ether.h>

#include "util.hh"

enum class MessageType: uint8_t {
	QUERY = 0x11,
	REPORT = 0x22
};

enum class RecordType: uint8_t {
	MODE_IS_INCLUDE = 1,
	MODE_IS_EXCLUDE = 2,
	CHANGE_TO_INCLUDE_MODE = 3,
	CHANGE_TO_EXCLUDE_MODE = 4,
	ALLOW_NEW_SOURCES = 5,
	BLOCK_OLD_SOURCES = 6
};

const bool INCLUDE = true;
const bool EXCLUDE = false;

const int LOCAL = 0;

inline bool is_state_change(const RecordType& r) {
	return r == RecordType::CHANGE_TO_INCLUDE_MODE or r == RecordType::CHANGE_TO_EXCLUDE_MODE
			or r == RecordType::ALLOW_NEW_SOURCES or r == RecordType::BLOCK_OLD_SOURCES;
}

inline bool is_current_state(const RecordType& r) {
	return r == RecordType::MODE_IS_INCLUDE or r == RecordType::MODE_IS_EXCLUDE;
}

inline RecordType MODE_IS_(bool include) {
	return include ? RecordType::MODE_IS_INCLUDE : RecordType::MODE_IS_EXCLUDE;
}

inline RecordType CHANGE_TO_(bool include) {
	return include ? RecordType::CHANGE_TO_INCLUDE_MODE : RecordType::CHANGE_TO_EXCLUDE_MODE;
}

inline const char* RecordType_to_string(const RecordType& r) {
	switch (r) {
		case RecordType::MODE_IS_INCLUDE:
			return "IS_IN";
		case RecordType::MODE_IS_EXCLUDE:
			return "IS_EX";
		case RecordType::CHANGE_TO_INCLUDE_MODE:
			return "TO_IN";
		case RecordType::CHANGE_TO_EXCLUDE_MODE:
			return "TO_EX";
		case RecordType::ALLOW_NEW_SOURCES:
			return "ALLOW";
		case RecordType::BLOCK_OLD_SOURCES:
			return "BLOCK";
		default:
			return "WTF?";
	}
}

inline const char* Include_to_string(bool include) {
	if (include) return "INCLUDE";
	else return "EXCLUDE";
}

const int default_headroom = sizeof(click_ether) + sizeof(click_ip);

// beware of Endianness!
const IPAddress GENERAL_QUERY_ADDRESS = 0x010000E0; // 224.0.0.1
const IPAddress REPORT_ADDRESS        = 0x160000E0; // 224.0.0.22

namespace defaults {
	const uint8_t ROBUSTNESS = 2;
	
	const seconds QUERY_INTERVAL = 8; // 125, but lowered for debugging purposes
	const MiniFloat QUERY_INTERVAL_CODE = MiniFloat(QUERY_INTERVAL);
	
	const centiseconds MAX_RESP_TIME = 10; // 100, but lowered for debugging purposes
	const MiniFloat MAX_RESP_CODE = MiniFloat(MAX_RESP_TIME);
	
	const seconds UNSOLICITED_REPORT_INTERVAL = 1;
	
	const centiseconds LAST_MEMBER_QUERY_INTERVAL = 10;
	const unsigned int LAST_MEMBER_QUERY_COUNT = ROBUSTNESS;
}
