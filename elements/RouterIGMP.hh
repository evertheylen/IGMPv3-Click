
#pragma once

#include "click/timer.hh"

#include "IGMP.hh"
#include "MCTable.hh"

CLICK_DECLS

class RouterIGMP: public IGMP {
public:
	const char *class_name() const	{ return "RouterIGMP"; }
	const char *port_count() const	{ return "-/="; }
	
	int configure(Vector<String>&, ErrorHandler*);
	
	void got_report(int port, Report* report, Packet* p);
	// TODO implement get_query
	
	int initialize(ErrorHandler * errh);

	void run_timer(Timer*);

private:
	RouterMCTable* table;
	Timer timer;
};

CLICK_ENDDECLS
