
#pragma once

#include <cstdint>

enum class MessageType: uint8_t {
	QUERY = 0x11,
	REPORT = 0x22
};

enum class RecordType: uint8_t {
	MODE_IS_INCLUDE = 1,
	MODE_IS_EXCLUDE = 2,
	CHANGE_TO_INCLUDE_MODE = 3,
	CHANGE_TO_EXCLUDE_MODE = 4
};

const bool INCLUDE = true;
const bool EXCLUDE = false;

inline bool is_state_change(RecordType& r) {
	return r == RecordType::CHANGE_TO_INCLUDE_MODE || r == RecordType::CHANGE_TO_EXCLUDE_MODE;
}

inline RecordType MODE_IS_(bool include) {
	return include ? RecordType::MODE_IS_INCLUDE : RecordType::MODE_IS_EXCLUDE;
}

inline RecordType CHANGE_TO_(bool include) {
	return include ? RecordType::CHANGE_TO_INCLUDE_MODE : RecordType::CHANGE_TO_EXCLUDE_MODE;
}

// beware of Endianness!
const IPAddress GENERAL_QUERY_ADDRESS = 0x010000E0; // 224.0.0.1
const IPAddress REPORT_ADDRESS        = 0x160000E0; // 224.0.0.22
