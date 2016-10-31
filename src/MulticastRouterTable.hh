
#pragma once

#include <click/element.hh>

class MulticastRouter;
class IGMPRouter;

CLICK_DECLS

class MulticastRouterTable: public Element {
public:
	MulticastRouterTable();
	
	const char *class_name() const	{ return "MulticastRouterTable"; }
	
	int configure(Vector<String>&, ErrorHandler*);
	
	friend class MulticastRouter;
	friend class IGMPRouter;
	
private:
	bool online = false;
};

CLICK_ENDDECLS
