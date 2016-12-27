#include "util.hh"

std::random_device rd;

std::vector<String> split(const String& s, char delim) {
	std::vector<String> v;
	String buf;
	
	for (char c: s) {
		if (c == delim) {
			if (buf.length() > 0) v.push_back(buf);
			buf = "";
		} else {
			buf += c;
		}
	}
	if (buf.length() > 0) v.push_back(buf);
	
	return v;
}

SourceState::SourceState(bool _include, const std::set<IPAddress>& _s): include(_include), sources(_s) {}

