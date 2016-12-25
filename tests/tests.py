
import unittest
import time

from base import *


# Actual tests
# ------------
    
class Simple(ClickTest):
    script = localdir("../scripts/ipnetwork.click")
    
    def test_single_exclude(self):
        self.write("client32/igmp.change_mode EXCLUDE 224.4.4.4")
        self.clients(online=["client32"], offline=["client31"])
        self.read_expect("router/mc_table.table", "224.4.4.4.*EXCLUDE")
    
    def test_simple_silent_exclude(self):
        self.write("client32/igmp.silent_change_mode EXCLUDE 224.4.4.4")
        self.clients(online=["client32"], offline=["client31"], timeout=general_query)
        self.read_expect("router/mc_table.table", "224.4.4.4.*EXCLUDE")
        
    def test_double_exclude(self):
        self.write("client21/igmp.change_mode EXCLUDE 224.4.4.4")
        self.write("client31/igmp.change_mode EXCLUDE 224.4.4.4")
        self.clients(online=["client21", "client31"], offline=["client22", "client32"])
    
    def test_tussentijdse(self):
        self.write("client21/igmp.change_mode EXCLUDE 224.4.4.4")
        self.clients(online=["client21"])
        self.write("client31/igmp.change_mode EXCLUDE 224.4.4.4")
        self.write("client22/igmp.change_mode EXCLUDE 224.4.4.4")
        self.clients(online=["client21", "client22", "client31"], offline=["client32"])
        self.write("client21/igmp.change_mode INCLUDE 224.4.4.4")
        self.clients(online=["client22", "client31"], offline=["client21", "client32"])
        self.write("client31/igmp.change_mode INCLUDE 224.4.4.4")
        self.clients(online=["client22"], offline=["client21", "client32", "client31"])
        self.write("client22/igmp.change_mode INCLUDE 224.4.4.4")
        self.clients(online=[], offline=["client21", "client22", "client32", "client31"])


def group(i):
    return "224\.{0}\.{0}\.{0}".format(i)

def source(i):
    return "192\.168\.1\.{0}".format(i)

class Advanced(ClickTest):
    script = localdir("../scripts/advanced_ipnetwork.click")
    
    # Possible senders (x -> 192.168.1.x), by group:
    # 224.1.1.1 --> {1, 2, 3}
    # 224.2.2.2 --> {4, 5, 6}
    # 224.3.3.3 --> {7, 8, 9}
    
    # Test the table on p32-31
    
    def test_include_allow(self):
        self.write("client21/igmp.allow 224.1.1.1 192.168.1.1")
        self.write("client21/igmp.allow 224.1.1.1 192.168.1.2")
        self.click.expect_exact("INCLUDE(192.168.1.1, 192.168.1.2)")
        self.clients(online=[("client21", group(1), source(1)),
                             ("client21", group(1), source(2))])
    
    def test_include_block(self):
        self.write("client21/igmp.allow 224.1.1.1 192.168.1.1 192.168.1.2 192.168.1.3")
        self.click.expect_exact("INCLUDE(192.168.1.1, 192.168.1.2, 192.168.1.3)")
        self.write("client21/igmp.block 224.1.1.1 192.168.1.3 192.168.1.1")
        self.click.expect_exact("INCLUDE(192.168.1.1, 192.168.1.2, 192.168.1.3)")
        self.expire()
        self.clients(online=[("client21", group(1), source(2))],
                     offline=[("client21", group(1), source(1)),
                              ("client21", group(1), source(3))])
    
    def test_include_to_ex(self):
        self.write("client21/igmp.allow 224.1.1.1 192.168.1.1 192.168.1.2")
        self.write("client21/igmp.change_mode EXCLUDE 224.1.1.1 192.168.1.1 192.168.1.3")
        self.click.expect_exact("EXCLUDE({192.168.1.1}, {192.168.1.3})")
        self.expire()
        self.clients(online=[("client21", group(1), source(2))],
                     offline=[("client21", group(1), source(1)),
                              ("client21", group(1), source(3))])
    
    def test_include_to_in(self):
        self.write("client21/igmp.allow 224.1.1.1 192.168.1.1 192.168.1.2")
        self.write("client21/igmp.silent_change_mode EXCLUDE 224.1.1.1")  # required for a clean TO_IN message
        self.write("client21/igmp.change_mode INCLUDE 224.1.1.1 192.168.1.3 192.168.1.1")
        self.click.expect_exact("INCLUDE(192.168.1.1, 192.168.1.2, 192.168.1.3)")
        self.expire()
        self.clients(online=[("client21", group(1), source(1)),
                             ("client21", group(1), source(3))],
                     offline=[("client21", group(1), source(2))])
    
        
    

if __name__ == '__main__':
    unittest.main()
