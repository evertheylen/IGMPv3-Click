
#include "Report.hh"
#include "util.hh"

ReportBuilder::ReportBuilder(uint16_t number_group_records, int tailroom) {
	if (tailroom == -1) tailroom = sizeof(GroupRecord) * number_group_records;
	packet = Packet::make(default_headroom, nullptr, sizeof(Report), tailroom);
	report = new (packet->data()) Report;
	report->number_group_records = hton_16(number_group_records);
	ptr = (unsigned char*) packet->data() + sizeof(Report);
}

GroupRecord* ReportBuilder::add_record(RecordType type, IPAddress multicast_address, std::initializer_list<IPAddress> sources, int extra_sources) {
	// make place
	uint16_t N = sources.size() + extra_sources;
	packet->put(sizeof(GroupRecord) + N*sizeof(IPAddress));
	
	// init new data
	GroupRecord* rec = new (ptr) GroupRecord;
	ptr += sizeof(GroupRecord);
	rec->type = type;
	rec->multicast_address = multicast_address;
	rec->N = hton_16(N);
	if (N > 0) memcpy(ptr, sources.begin(), sizeof(IPAddress) * N);
	ptr += (sizeof(IPAddress) * N);
	
	return rec;
}
