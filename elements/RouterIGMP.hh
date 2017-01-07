
#pragma once

#include "click/timer.hh"

#include "IGMP.hh"
#include "MCTable.hh"

CLICK_DECLS

class RouterIGMP: public IGMP {
public:
	const char *class_name() const	{ return "RouterIGMP"; }
	const char *port_count() const	{ return "-/="; }
	
	void got_report(int port, Report* report, Packet* p);
	
	int initialize(ErrorHandler * errh);

	static void run_send_gq_timer(Timer*, void* user_data);

private:
	Timer send_gq_timer;
};

CLICK_ENDDECLS
