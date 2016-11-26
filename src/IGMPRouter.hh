
#pragma once

#include "IGMP.hh"

CLICK_DECLS

class IGMPRouter: public IGMP {
public:
	const char *class_name() const	{ return "IGMPRouter"; }
	const char *port_count() const	{ return "-/1"; }
	const char *processing() const	{ return PUSH; }

	int initialize(ErrorHandler * errh);

	void run_timer(Timer*);

private:
	Timer timer;
};

CLICK_ENDDECLS
