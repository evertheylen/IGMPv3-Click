
#include <clicknet/ip.h>

#include "Report.hh"
#include "util.hh"

ReportBuilder::ReportBuilder(uint16_t number_group_records, int tailroom) {
	if (tailroom == -1) tailroom = sizeof(GroupRecord) * number_group_records;
	packet = Packet::make(default_headroom, nullptr, sizeof(Report), tailroom);
	Report* report = new (packet->data()) Report;
	report->number_group_records = hton_16(number_group_records);
}

GroupRecord* ReportBuilder::add_record(RecordType type, IPAddress multicast_address, std::initializer_list<IPAddress> sources, int extra_sources) {
	// make place
	uint16_t N = sources.size() + extra_sources;
	unsigned char* ptr = packet->end_data();
	packet = packet->put(sizeof(GroupRecord) + N*sizeof(IPAddress));
	
	// init new data
	GroupRecord* rec = new (ptr) GroupRecord;
	rec->type = type;
	rec->multicast_address = multicast_address;
	rec->N = hton_16(N);
	if (N > 0) memcpy(ptr + sizeof(GroupRecord), sources.begin(), sizeof(IPAddress) * N);
	
	return rec;
}

void ReportBuilder::set_checksum() {
	report()->checksum = 0;
	report()->checksum = click_in_cksum(packet->data(), packet->length());
}
