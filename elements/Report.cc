
#include <clicknet/ip.h>

#include "Report.hh"
#include "util.hh"

ReportBuilder::ReportBuilder(uint16_t number_group_records, int tailroom) {
	// by default, make room for a grouprecord with one source
	if (tailroom == -1) tailroom = (sizeof(GroupRecord) + sizeof(IPAddress)) * number_group_records;
	packet = Packet::make(default_headroom, nullptr, sizeof(Report), tailroom);
	Report* report = new (packet->data()) Report;
	report->number_group_records = hton_16(number_group_records);
}

GroupRecord* ReportBuilder::add_record(RecordType type, IPAddress multicast_address, int sources) {
	// make place
	uint16_t N = sources;
	unsigned int extra_space = sizeof(GroupRecord) + N*sizeof(IPAddress);
	packet = packet->put(extra_space);
	uint8_t* ptr = ((uint8_t*) packet->end_data()) - extra_space;
	
	// init new data
	GroupRecord* rec = new (ptr) GroupRecord;
	rec->type = type;
	rec->multicast_address = multicast_address;
	rec->N = hton_16(N);
	
	return rec;
}

void ReportBuilder::prepare() {
	packet->set_dst_ip_anno(REPORT_ADDRESS);
	report()->checksum = 0;
	report()->checksum = click_in_cksum(packet->data(), packet->length());
}
