
// Advanced (and thus edited) version of ipnetwork.click
// Defines multiple sources

// See below for the Group addresses and sources configuration

require(library routers/definitions.click)
require(library routers/server.click);
require(library routers/client.click);
require(library routers/router.click);

// Address configuration
AddressInfo(router_server_network_address 192.168.1.254/24 00:50:BA:85:84:A1);

AddressInfo(router_client_network1_address 192.168.2.254/24 00:50:BA:85:84:B1);
AddressInfo(client21_address 192.168.2.1/24 00:50:BA:85:84:B2);
AddressInfo(client22_address 192.168.2.2/24 00:50:BA:85:84:B3);

AddressInfo(router_client_network2_address 192.168.3.254/24 00:50:BA:85:84:C1);
AddressInfo(client31_address 192.168.3.1/24 00:50:BA:85:84:C2);
AddressInfo(client32_address 192.168.3.2/24 00:50:BA:85:84:C3);

// Host, router and switch instantiation
client21 :: Client(client21_address, router_client_network1_address);
client22 :: Client(client22_address, router_client_network1_address);
client31 :: Client(client31_address, router_client_network2_address);
client32 :: Client(client32_address, router_client_network2_address);
router :: Router(router_server_network_address, router_client_network1_address, router_client_network2_address);
server_network :: ListenEtherSwitch;
client_network1 :: ListenEtherSwitch;
client_network2 :: ListenEtherSwitch;


// Connect the hosts and routers to the network switches

client21
	-> client_network1
	-> client21

client21[1]
	-> IPPrint("client21 -- received a packet") 
	-> Discard

client22
	-> [1]client_network1[1]
	-> client22

client22[1]
	-> IPPrint("client22 --received a packet") 
	-> Discard

client31
	-> client_network2
	-> client31

client31[1]
	-> IPPrint("client31 -- received a packet") 
	-> Discard

client32
	-> [1]client_network2[1]
	-> client32

client32[1]
	-> IPPrint("client32 -- received a packet") 
	-> Discard

router
	-> [0]server_network[0] // 0, so you can easily add new sources
	-> router

router[1]
	-> [2]client_network1[2]
	-> [1]router

router[2]
	-> [2]client_network2[2]
	-> [2]router

router[3]
	-> IPPrint("router -- received a packet")
	-> Discard

// In every network, create pcap dump files
server_network[10]
	-> ToDump("server_network.pcap");

client_network1[3]
	-> ToDump("client_network1.pcap");

client_network2[3]
	-> ToDump("client_network2.pcap");

// Sources
// =======

elementclass Source {
	$group, $address |
	
	server :: Server($address, router_server_network_address);
	
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

AddressInfo(a1 192.168.1.1/24 00:50:BA:85:84:A2);
AddressInfo(a2 192.168.1.2/24 00:50:BA:85:84:A3);
AddressInfo(a3 192.168.1.3/24 00:50:BA:85:84:A4);
AddressInfo(a4 192.168.1.4/24 00:50:BA:85:84:A5);
AddressInfo(a5 192.168.1.5/24 00:50:BA:85:84:A6);
AddressInfo(a6 192.168.1.6/24 00:50:BA:85:84:A7);
AddressInfo(a7 192.168.1.7/24 00:50:BA:85:84:A8);
AddressInfo(a8 192.168.1.8/24 00:50:BA:85:84:A9);
AddressInfo(a9 192.168.1.9/24 00:50:BA:85:84:AA);

s1::Source(g1, a1) -> [1] server_network [1] -> s1
s2::Source(g1, a2) -> [2] server_network [2] -> s2
s3::Source(g1, a3) -> [3] server_network [3] -> s3

s4::Source(g2, a4) -> [4] server_network [4] -> s4
s5::Source(g2, a5) -> [5] server_network [5] -> s5
s6::Source(g2, a6) -> [6] server_network [6] -> s6

s7::Source(g3, a7) -> [7] server_network [7] -> s7
s8::Source(g3, a8) -> [8] server_network [8] -> s8
s9::Source(g3, a9) -> [9] server_network [9] -> s9
