// Output configuration: 
//
// Packets for the network are put on output 0
// Packets for the host are put on output 1

elementclass Client {
	$address, $gateway |
	
	// IP lookup table
	// ===============
	
	ip :: Strip(14)
		-> CheckIPHeader()
		-> rt :: StaticIPLookup(
					224.0.0.0/4 2,  // Class D
					$address:ip/32 0,
					$address:ipnet 0,
					0.0.0.0/0.0.0.0 $gateway 1,
					)
		-> [1]output;
	
	
	// Multicast stuff
	// ===============
		
	mc_table :: MulticastTable;
	igmp :: IGMP(mc_table);
	mc :: Multicast(mc_table);
	igmp_class::IPClassifier(
		ip proto igmp and dst host 224.0.0.1,
		ip proto igmp,
		-);
	
	rt[2] 
		-> mc
		-> [1] output // mc has one port: this host
	
	igmp_class[1] // IGMP messages
		-> DropBroadcasts // stops messages from the other client on the same subnet
		-> strip_igmp::StripIPHeader
		-> igmp
		-> IPEncap(2, $address, DST DST_ANNO, TTL 1)
		//-> IPPrint("Client igmp sent something")
		-> arpq :: ARPQuerier($address)
		-> output
	
	igmp_class[0] // IGMP messages (from 224.0.0.1) should skip the DropBroadcasts
		-> strip_igmp
	
	igmp_class[2] // All the rest (data)
		-> ip
	
	// The rest of the IP stuff
	// ========================
	
	rt[1]
		-> DropBroadcasts
		-> ipgw :: IPGWOptions($address)
		-> FixIPSrc($address)
		-> ttl :: DecIPTTL
		-> frag :: IPFragmenter(1500)
		-> arpq
	
	ipgw[1]
		-> ICMPError($address, parameterproblem)
		-> output;
	
	ttl[1]
		-> ICMPError($address, timeexceeded)
		-> output; 

	frag[1]
		-> ICMPError($address, unreachable, needfrag)
		-> output;
	
	
	// Ethernet stuff
	// ==============
	
	input
		-> HostEtherFilter($address)
		-> in_cl :: Classifier(12/0806 20/0001, 12/0806 20/0002, 12/0800)
		-> arp_res :: ARPResponder($address)
		-> output;

	in_cl[1]
		-> [1]arpq;
	
	in_cl[2]
		-> igmp_class;
}
