// Advanced (and thus edited) version of ipnetwork.click
// Defines multiple sources, with multiple routers!

// See below for the Group addresses and sources configuration

require(library routers/definitions.click);
require(library routers/server.click);
require(library routers/client.click);
require(library routers/router.click);

elementclass SubNetwork {
	$router_server_network_address, $router_client_network1_address, $router_client_network2_address |
	// Simple EtherSwitches for now (how to put in different files?)
	server_network :: EtherSwitch;
	client_network1 :: EtherSwitch;
	client_network2 :: EtherSwitch;
	
	router :: Router($router_server_network_address, $router_client_network1_address, $router_client_network2_address);
	
	input[0] -> [0]server_network[0] -> [0]output;
	router[0] -> [1]server_network[1] -> [0]router;
	
	router[1] -> [0]client_network1[0] -> [1]router;
	router[2] -> [0]client_network2[0] -> [2]router;
	router[3]
		-> IPPrint("some router -- received a packet")
		-> Discard
	
	input[1] -> [1]client_network1[1] -> [1]output;
	input[2] -> [2]client_network1[2] -> [2]output;
	
	input[3] -> [1]client_network2[1] -> [3]output;
	input[4] -> [2]client_network2[2] -> [4]output;
}

// Address configuration and instantiation
// =======================================

// Root
AddressInfo(root_router_server_network_address 192.255.1.254/24 00:50:BA:85:84:A1);
AddressInfo(root_router_client_network1_address 192.255.2.254/24 00:50:BA:85:84:B1);
AddressInfo(root_client21_address 192.255.2.1/24 00:50:BA:85:84:B2);
AddressInfo(root_client22_address 192.255.2.2/24 00:50:BA:85:84:B3);
AddressInfo(root_router_client_network2_address 192.255.3.254/24 00:50:BA:85:84:C1);
AddressInfo(root_client31_address 192.255.3.1/24 00:50:BA:85:84:C2);
AddressInfo(root_client32_address 192.255.3.2/24 00:50:BA:85:84:C3);

// Subnetwork 1
AddressInfo(n1_router_server_network_address 192.1.1.254/24 00:50:BA:85:84:A1);
AddressInfo(n1_router_client_network1_address 192.1.2.254/24 00:50:BA:85:84:B1);
AddressInfo(n1_client21_address 192.1.2.1/24 00:50:BA:85:84:B2);
AddressInfo(n1_client22_address 192.1.2.2/24 00:50:BA:85:84:B3);
AddressInfo(n1_router_client_network2_address 192.1.3.254/24 00:50:BA:85:84:C1);
AddressInfo(n1_client31_address 192.1.3.1/24 00:50:BA:85:84:C2);
AddressInfo(n1_client32_address 192.1.3.2/24 00:50:BA:85:84:C3);
n1_client21 :: Client(n1_client21_address, n1_router_client_network1_address);
n1_client22 :: Client(n1_client22_address, n1_router_client_network1_address);
n1_client31 :: Client(n1_client31_address, n1_router_client_network2_address);
n1_client32 :: Client(n1_client32_address, n1_router_client_network2_address);

// Subnetwork 2
AddressInfo(n2_router_server_network_address 192.2.1.254/24 00:50:BA:85:84:A1);
AddressInfo(n2_router_client_network1_address 192.2.2.254/24 00:50:BA:85:84:B1);
AddressInfo(n2_client21_address 192.2.2.1/24 00:50:BA:85:84:B2);
AddressInfo(n2_client22_address 192.2.2.2/24 00:50:BA:85:84:B3);
AddressInfo(n2_router_client_network2_address 192.2.3.254/24 00:50:BA:85:84:C1);
AddressInfo(n2_client31_address 192.2.3.1/24 00:50:BA:85:84:C2);
AddressInfo(n2_client32_address 192.2.3.2/24 00:50:BA:85:84:C3);
n2_client21 :: Client(n2_client21_address, n2_router_client_network1_address);
n2_client22 :: Client(n2_client22_address, n2_router_client_network1_address);
n2_client31 :: Client(n2_client31_address, n2_router_client_network2_address);
n2_client32 :: Client(n2_client32_address, n2_router_client_network2_address);

// Subnetwork 3
AddressInfo(n3_router_server_network_address 192.16.1.254/24 00:50:BA:85:84:A1);
AddressInfo(n3_router_client_network1_address 192.16.2.254/24 00:50:BA:85:84:B1);
AddressInfo(n3_client21_address 192.16.2.1/24 00:50:BA:85:84:B2);
AddressInfo(n3_client22_address 192.16.2.2/24 00:50:BA:85:84:B3);
AddressInfo(n3_router_client_network2_address 192.16.3.254/24 00:50:BA:85:84:C1);
AddressInfo(n3_client31_address 192.16.3.1/24 00:50:BA:85:84:C2);
AddressInfo(n3_client32_address 192.16.3.2/24 00:50:BA:85:84:C3);
n3_client21 :: Client(n3_client21_address, n3_router_client_network1_address);
n3_client22 :: Client(n3_client22_address, n3_router_client_network1_address);
n3_client31 :: Client(n3_client31_address, n3_router_client_network2_address);
n3_client32 :: Client(n3_client32_address, n3_router_client_network2_address);

// Subnetwork 4
AddressInfo(n4_router_server_network_address 192.17.1.254/24 00:50:BA:85:84:A1);
AddressInfo(n4_router_client_network1_address 192.17.2.254/24 00:50:BA:85:84:B1);
AddressInfo(n4_client21_address 192.17.2.1/24 00:50:BA:85:84:B2);
AddressInfo(n4_client22_address 192.17.2.2/24 00:50:BA:85:84:B3);
AddressInfo(n4_router_client_network2_address 192.17.3.254/24 00:50:BA:85:84:C1);
AddressInfo(n4_client31_address 192.17.3.1/24 00:50:BA:85:84:C2);
AddressInfo(n4_client32_address 192.17.3.2/24 00:50:BA:85:84:C3);
n4_client21 :: Client(n4_client21_address, n4_router_client_network1_address);
n4_client22 :: Client(n4_client22_address, n4_router_client_network1_address);
n4_client31 :: Client(n4_client31_address, n4_router_client_network2_address);
n4_client32 :: Client(n4_client32_address, n4_router_client_network2_address);


// Subnetworks
// ===========

AddressInfo(root_address 192.255.0.0/12);
AddressInfo(n12_address 192.0.0.0/12);
AddressInfo(n34_address 192.16.0.0/12);
root :: SubNetwork(root_router_server_network_address, root_router_client_network1_address, root_router_client_network2_address);
n1 :: SubNetwork(n1_router_server_network_address, n1_router_client_network1_address, n1_router_client_network2_address);
n2 :: SubNetwork(n2_router_server_network_address, n2_router_client_network1_address, n2_router_client_network2_address);
n3 :: SubNetwork(n3_router_server_network_address, n3_router_client_network1_address, n3_router_client_network2_address);
n4 :: SubNetwork(n4_router_server_network_address, n4_router_client_network1_address, n4_router_client_network2_address);

// Connect it the subnetworks
root[1] -> [0]n1[0] -> [1]root
root[2] -> [0]n2[0] -> [2]root
root[3] -> [0]n3[0] -> [3]root
root[4] -> [0]n4[0] -> [4]root

source_network :: EtherSwitch;
root[0] -> [0]source_network[0] -> [0]root

// Connect the clients
// ===================

// n1
n1[1] -> n1_client21 -> [1]n1
n1_client21[1] -> IPPrint("n1_client21 -- received a packet") -> Discard
n1[2] -> n1_client22 -> [2]n1
n1_client22[1] -> IPPrint("n1_client22 -- received a packet") -> Discard
n1[3] -> n1_client31 -> [3]n1
n1_client31[1] -> IPPrint("n1_client31 -- received a packet") -> Discard
n1[4] -> n1_client32 -> [4]n1
n1_client32[1] -> IPPrint("n1_client32 -- received a packet") -> Discard

// n2
n2[1] -> n2_client21 -> [1]n2
n2_client21[1] -> IPPrint("n2_client21 -- received a packet") -> Discard
n2[2] -> n2_client22 -> [2]n2
n2_client22[1] -> IPPrint("n2_client22 -- received a packet") -> Discard
n2[3] -> n2_client31 -> [3]n2
n2_client31[1] -> IPPrint("n2_client31 -- received a packet") -> Discard
n2[4] -> n2_client32 -> [4]n2
n2_client32[1] -> IPPrint("n2_client32 -- received a packet") -> Discard

// n3
n3[1] -> n3_client21 -> [1]n3
n3_client21[1] -> IPPrint("n3_client21 -- received a packet") -> Discard
n3[2] -> n3_client22 -> [2]n3
n3_client22[1] -> IPPrint("n3_client22 -- received a packet") -> Discard
n3[3] -> n3_client31 -> [3]n3
n3_client31[1] -> IPPrint("n3_client31 -- received a packet") -> Discard
n3[4] -> n3_client32 -> [4]n3
n3_client32[1] -> IPPrint("n3_client32 -- received a packet") -> Discard

// n4
n4[1] -> n4_client21 -> [1]n4
n4_client21[1] -> IPPrint("n4_client21 -- received a packet") -> Discard
n4[2] -> n4_client22 -> [2]n4
n4_client22[1] -> IPPrint("n4_client22 -- received a packet") -> Discard
n4[3] -> n4_client31 -> [3]n4
n4_client31[1] -> IPPrint("n4_client31 -- received a packet") -> Discard
n4[4] -> n4_client32 -> [4]n4
n4_client32[1] -> IPPrint("n4_client32 -- received a packet") -> Discard


// Sources
// =======

elementclass Source {
	$group, $address |
	
	server :: Server($address, root_router_server_network_address);
	
	// Generate traffic for the multicast server.
	RatedSource("data", 1, -1, true)
		-> DynamicUDPIPEncap($address:ip, 1234, $group:ip, 1234)
		-> EtherEncap(0x0800, $address:eth, $address:eth) /// The MAC addresses here should be from the multicast_server to get past the HostEtherFilter. This way we can reuse the input from the network for the applications.
		//-> IPPrint(" -- transmitted a UDP packet")
		-> server
	
	input -> server -> output;
	
	server[1] -> Discard;
}

AddressInfo(g1 224.1.1.1)
AddressInfo(g2 224.2.2.2)
AddressInfo(g3 224.3.3.3)
AddressInfo(g4 224.4.4.4)
AddressInfo(g5 224.5.5.5)
AddressInfo(g6 224.6.6.6)

AddressInfo(a1 193.168.1.1/24 00:50:BA:85:84:A2);
AddressInfo(a2 193.168.1.2/24 00:50:BA:85:84:A3);
AddressInfo(a3 193.168.1.3/24 00:50:BA:85:84:A4);
AddressInfo(a4 193.168.1.4/24 00:50:BA:85:84:A5);
AddressInfo(a5 193.168.1.5/24 00:50:BA:85:84:A6);
AddressInfo(a6 193.168.1.6/24 00:50:BA:85:84:A7);
AddressInfo(a7 193.168.1.7/24 00:50:BA:85:84:A8);
AddressInfo(a8 193.168.1.8/24 00:50:BA:85:84:A9);
AddressInfo(a9 193.168.1.9/24 00:50:BA:85:84:AA);

s1::Source(g1, a1) -> [1] source_network [1] -> s1
s2::Source(g1, a2) -> [2] source_network [2] -> s2
s3::Source(g1, a3) -> [3] source_network [3] -> s3

s4::Source(g2, a4) -> [4] source_network [4] -> s4
s5::Source(g2, a5) -> [5] source_network [5] -> s5
s6::Source(g2, a6) -> [6] source_network [6] -> s6

s7::Source(g3, a7) -> [7] source_network [7] -> s7
s8::Source(g3, a8) -> [8] source_network [8] -> s8
s9::Source(g3, a9) -> [9] source_network [9] -> s9
