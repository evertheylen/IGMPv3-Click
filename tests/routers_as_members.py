
import unittest
import time

from base import *

def group(i):
    return "224\.{0}\.{0}\.{0}".format(i)

def source(i):
    return "193\.168\.1\.{0}".format(i)

n2s1 = ("n2_client21", group(1), source(1))
n2s2 = ("n2_client21", group(1), source(2))
n2s3 = ("n2_client21", group(1), source(3))
n3s1 = ("n3_client32", group(1), source(1))
n3s2 = ("n3_client32", group(1), source(2))
n3s3 = ("n3_client32", group(1), source(3))
n4s1 = ("n4_client22", group(1), source(1))
n4s2 = ("n4_client22", group(1), source(2))
n4s3 = ("n4_client22", group(1), source(3))

class Routers(ClickTest):
    script = localdir("../scripts/routers_ipnetwork.click")
    
    def test_exclude(self):
        self.write("n3_client32/igmp.change_mode EXCLUDE 224.1.1.1 193.168.1.1")
        self.read_expect("n3/router/mc_table.table", "224.1.1.1.*EXCLUDE\({}, {193.168.1.1}\)")
        self.read_expect("root/router/mc_table.table", "224.1.1.1.*EXCLUDE\({}, {193.168.1.1}\)")
        
        self.write("n4_client22/igmp.change_mode EXCLUDE 224.1.1.1 193.168.1.2")
        self.read_expect("n4/router/mc_table.table", "224.1.1.1.*EXCLUDE\({}, {193.168.1.2}\)")
        # It might seem weird that the first set of the EXCLUDE is one of the two sources here.
        # While a intuitive explanation is hard to find, it is correct according to the RFC (see p31)
        # Whether it is 1 or 2 depends on who sends first.
        self.read_expect("root/router/mc_table.table", "224.1.1.1.*EXCLUDE\({193.168.1.[12]}, {}\)")
        self.clients(online=[n3s2, n3s3, n4s1, n4s3], offline=[n3s1, n4s2])
        
    def test_include(self):
        self.write("n3_client32/igmp.change_mode INCLUDE 224.1.1.1 193.168.1.1")
        self.read_expect("n3/router/mc_table.table", "224.1.1.1.*INCLUDE\(193.168.1.1\)")
        self.write("n2_client21/igmp.change_mode INCLUDE 224.1.1.1 193.168.1.2")
        self.read_expect("n2/router/mc_table.table", "224.1.1.1.*INCLUDE\(193.168.1.2\)")
        self.clients(online=[n3s1, n2s2])
        self.read_expect("root/router/mc_table.table", 
                        "Interface 1.*224.1.1.1.*INCLUDE\(193.168.1.2\).*Interface 2.*224.1.1.1.*INCLUDE\(193.168.1.1\)")
    
    def test_both(self):
        self.write("n3_client32/igmp.change_mode INCLUDE 224.1.1.1 193.168.1.1")
        self.read_expect("n3/router/mc_table.table", "224.1.1.1.*INCLUDE\(193.168.1.1\)")
        self.write("n2_client21/igmp.change_mode INCLUDE 224.1.1.1 193.168.1.2")
        self.read_expect("n2/router/mc_table.table", "224.1.1.1.*INCLUDE\(193.168.1.2\)")
        self.write("n4_client22/igmp.change_mode EXCLUDE 224.1.1.1 193.168.1.2 193.168.1.3")
        self.read_expect("n4/router/mc_table.table", "224.1.1.1.*EXCLUDE")
        self.clients(online=[n3s1, n2s2, n4s1], offline=[n4s2, n4s3])
        #self.read_expect("root/router/mc_table.table", 
                        #"Interface 1.*224.1.1.1.*INCLUDE\(193.168.1.2\).*Interface 2.*224.1.1.1.*INCLUDE\(193.168.1.1\)")

