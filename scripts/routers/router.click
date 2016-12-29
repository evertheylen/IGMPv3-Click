// Router with three interfaces
// The input/output configuration is as follows:
//
// Input:
//	[0]: packets received on the 192.168.1.0/24 network
//	[1]: packets received on the 192.168.2.0/24 network
//	[2]: packets received on the 192.168.3.0/24 network
//
// Output:
//	[0]: packets sent to the 192.168.1.0/24 network
//	[1]: packets sent to the 192.168.2.0/24 network
//	[2]: packets sent to the 192.168.3.0/24 network
//  [3]: packets destined for the router itself

elementclass Router {
	$server_address, $client1_address, $client2_address |

	// Important ARP / IP elements
	// ===========================
	
	// Shared IP input path and routing table
	ip :: Strip(14)
		-> CheckIPHeader
		-> rt :: StaticIPLookup(
					$server_address:ip/32 0,
					$client1_address:ip/32 0,
					$client2_address:ip/32 0,
					$server_address:ipnet 1,
					$client1_address:ipnet 2,
					$client2_address:ipnet 3,
					224.0.0.0/4 4,  // Class D (for multicast messages)
					);
	
	// ARP responses are copied to each ARPQuerier and the host.
	arpt :: Tee (3);
	
	
	// Important Multicast / IGMP elements
	// ===================================
	
	mc_table :: MCTable;
	igmp :: RouterIGMP(mc_table)
	mc :: RouterMC(mc_table);
	
	rt[4] -> mc
	
	
	// Input and output paths
	// ======================
	
	// interface 0 (server)
	input
		-> HostEtherFilter($server_address)
		-> server_class :: Classifier(12/0806 20/0001, 12/0806 20/0002, -)
		-> ARPResponder($server_address)
		-> output;

	server_arpq :: ARPQuerier($server_address)
		-> [0]output;

	server_class[1]
		-> arpt
		-> [1]server_arpq;

	server_class[2]
		-> Paint(1)
		-> server_igmp_class::IPClassifier(ip proto igmp, -)
		-> StripIPHeader
		-> [0] igmp
		
	server_igmp_class[1] -> ip;
	
	
	// interface 1 (client1)
	input[1]
		-> HostEtherFilter($client1_address)
		-> client1_class :: Classifier(12/0806 20/0001, 12/0806 20/0002, -)
		-> ARPResponder($client1_address)
		-> [1]output;

	client1_arpq :: ARPQuerier($client1_address)
		-> [1]output;

	client1_class[1]
		-> arpt[1]
		-> [1]client1_arpq;

	client1_class[2]
		-> Paint(2)
		-> client1_igmp_class::IPClassifier(ip proto igmp, -)
		-> StripIPHeader
		-> [1] igmp
		
	client1_igmp_class[1] -> ip;
	
	
	// interface 2 (client2)
	input[2]
		-> HostEtherFilter($client2_address)
		-> client2_class :: Classifier(12/0806 20/0001, 12/0806 20/0002, -)
		-> ARPResponder($client2_address)
		-> [2]output;

	client2_arpq :: ARPQuerier($client2_address)
		-> [2]output;

	client2_class[1]
		-> arpt[2]
		-> [1]client2_arpq;

	client2_class[2]
		-> Paint(3)
		-> client2_igmp_class::IPClassifier(ip proto igmp, -)
		-> StripIPHeader
		-> [2] igmp
		
	client2_igmp_class[1] -> ip;
	
	
	// Local delivery
	rt[0]
		-> [3]output
	
	// Forwarding paths per interface
	// ==============================
	
	// interface 0 (server)
	mc[0]
		-> server_ttl :: DecIPTTL
	
	rt[1]
		-> DropBroadcasts
		-> server_paint :: PaintTee(1)
		-> server_ipgw :: IPGWOptions($server_address)
		-> FixIPSrc($server_address)
		-> server_ttl
		-> server_frag :: IPFragmenter(1500)
		-> server_arpq;
	
	igmp[0]
		-> IPEncap(2, $server_address, DST DST_ANNO, TTL 1)
		-> server_arpq
	
	server_paint[1]
		-> ICMPError($server_address, redirect, host)
		-> rt;

	server_ipgw[1]
		-> ICMPError($server_address, parameterproblem)
		-> rt;

	server_ttl[1]
		-> ICMPError($server_address, timeexceeded)
		-> rt;

	server_frag[1]
		-> ICMPError($server_address, unreachable, needfrag)
		-> rt;
	
	
	// interface 1 (client1)
	mc[1]
		-> client1_ttl :: DecIPTTL
		
	rt[2]
		-> DropBroadcasts
		-> client1_paint :: PaintTee(2)
		-> client1_ipgw :: IPGWOptions($client1_address)
		-> FixIPSrc($client1_address)
		-> client1_ttl
		-> client1_frag :: IPFragmenter(1500)
		-> client1_arpq;
	
	igmp[1]
		-> IPEncap(2, $client1_address, DST DST_ANNO, TTL 1)
		-> client1_arpq
	
	client1_paint[1]
		-> ICMPError($client1_address, redirect, host)
		-> rt;

	client1_ipgw[1]
		-> ICMPError($client1_address, parameterproblem)
		-> rt;

	client1_ttl[1]
		-> ICMPError($client1_address, timeexceeded)
		-> rt;

	client1_frag[1]
		-> ICMPError($client1_address, unreachable, needfrag)
		-> rt;
	
		
	// interface 2 (client2)
	mc[2]
		-> client2_ttl :: DecIPTTL
	
	rt[3]
		-> DropBroadcasts
		-> client2_paint :: PaintTee(3)
		-> client2_ipgw :: IPGWOptions($client2_address)
		-> FixIPSrc($client2_address)
		-> client2_ttl
		-> client2_frag :: IPFragmenter(1500)
		-> client2_arpq;
	
	igmp[2]
		-> IPEncap(2, $client2_address, DST DST_ANNO, TTL 1)
		-> client2_arpq
	
	client2_paint[1]
		-> ICMPError($client2_address, redirect, host)
		-> rt;

	client2_ipgw[1]
		-> ICMPError($client2_address, parameterproblem)
		-> rt;

	client2_ttl[1]
		-> ICMPError($client2_address, timeexceeded)
		-> rt;

	client2_frag[1]
		-> ICMPError($client2_address, unreachable, needfrag)
		-> rt;
}
