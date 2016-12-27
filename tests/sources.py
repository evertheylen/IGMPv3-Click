
import unittest
import time

from base import *

def group(i):
    return "224\.{0}\.{0}\.{0}".format(i)

def source(i):
    return "192\.168\.1\.{0}".format(i)

s1 = ("client21", group(1), source(1))
s2 = ("client21", group(1), source(2))
s3 = ("client21", group(1), source(3))

# Possible senders (x -> 192.168.1.x), by group:
# 224.1.1.1 --> {1, 2, 3}
# 224.2.2.2 --> {4, 5, 6}
# 224.3.3.3 --> {7, 8, 9}


# Test the table on p32-31
# ========================

class Sources(ClickTest):
    script = localdir("../scripts/sources_ipnetwork.click")


class Include(Sources):
    # Old State = Include
    
    def test_allow(self):
        self.write("client21/igmp.change_sources ALLOW 224.1.1.1 192.168.1.1")
        self.write("client21/igmp.change_sources ALLOW 224.1.1.1 192.168.1.2")
        self.click.expect_exact("INCLUDE(192.168.1.1, 192.168.1.2)")
        self.clients(online=[s1, s2])
    
    def test_block(self):
        self.write("client21/igmp.change_sources ALLOW 224.1.1.1 192.168.1.1 192.168.1.2 192.168.1.3")
        self.click.expect_exact("INCLUDE(192.168.1.1, 192.168.1.2, 192.168.1.3)")
        self.write("client21/igmp.change_sources BLOCK 224.1.1.1 192.168.1.3 192.168.1.1")
        self.click.expect_exact("INCLUDE(192.168.1.1, 192.168.1.2, 192.168.1.3)")
        self.expire()
        self.clients(online=[s2], offline=[s1, s3])
    
    def test_to_ex(self):
        self.write("client21/igmp.change_sources ALLOW 224.1.1.1 192.168.1.1 192.168.1.2")
        self.write("client21/igmp.change_mode EXCLUDE 224.1.1.1 192.168.1.1 192.168.1.3")
        self.click.expect_exact("EXCLUDE({192.168.1.1}, {192.168.1.3})")
        self.expire()
        self.clients(online=[s2], offline=[s1, s3])
    
    def test_to_in(self):
        self.write("client21/igmp.change_sources ALLOW 224.1.1.1 192.168.1.1 192.168.1.2")
        self.write("client21/igmp.silent_change_mode EXCLUDE 224.1.1.1")  # required for a clean TO_IN message
        self.write("client21/igmp.change_mode INCLUDE 224.1.1.1 192.168.1.3 192.168.1.1")
        self.click.expect_exact("INCLUDE(192.168.1.1, 192.168.1.2, 192.168.1.3)")
        self.expire()
        self.clients(online=[s1, s3], offline=[s2])
    

class Exclude(Sources):
    # Old State = Exclude
    
    def setUp(self):
        super().setUp()
        self.write("client21/igmp.change_sources ALLOW 224.1.1.1 192.168.1.1")
        self.write("client21/igmp.change_mode EXCLUDE 224.1.1.1 192.168.1.1 192.168.1.2 192.168.1.3")
        self.click.expect_exact("EXCLUDE({192.168.1.1}, {192.168.1.2, 192.168.1.3})")
        # All tests start with EXCLUDE({1}, {2, 3})
        
    def test_allow(self):
        self.write("client21/igmp.change_sources ALLOW 224.1.1.1 192.168.1.1 192.168.1.3")
        self.click.expect_exact("EXCLUDE({192.168.1.1, 192.168.1.3}, {192.168.1.2})")
        self.clients(online=[s1, s3], offline=[s2])
    
    def test_block(self):
        self.write("client21/igmp.change_sources BLOCK 224.1.1.1 192.168.1.1 192.168.1.3")
        # EX(X={1}, Y={2, 3}) + BLOCK(A={1, 3})
        #     = EX(X+(A-Y), Y) = EX({1}, {2, 3})
        self.click.expect_exact("EXCLUDE({192.168.1.1}, {192.168.1.2, 192.168.1.3})")
        self.expire()
        self.clients(online=[], offline=[s1, s2, s3])
    
    def test_to_ex(self):
        self.write("client21/igmp.silent_change_mode INCLUDE 224.1.1.1")  # needed for clean TO_EX message
        self.write("client21/igmp.change_mode EXCLUDE 224.1.1.1 192.168.1.1 192.168.1.2")
        self.click.expect_exact("TO_EX(192.168.1.1, 192.168.1.2)")
        # EX(X={1}, Y={2, 3}) + TO_EX(A={1, 2})
        #     = EX(A-Y, Y*A) = EX({1, 3}, {1})
        self.click.expect_exact("EXCLUDE({192.168.1.1}, {192.168.1.2})")
        self.expire()
        self.clients(online=[s3], offline=[s1, s2])
    
    def test_to_in(self):
        self.write("client21/igmp.change_mode INCLUDE 224.1.1.1 192.168.1.1 192.168.1.2")
        # EX(X={1}, Y={2, 3}) + TO_IN(A={1, 2})
        #     = EX(X+A, Y-A) = EX({1, 2}, {3})
        self.click.expect_exact("EXCLUDE({192.168.1.1, 192.168.1.2}, {192.168.1.3})")
        self.expire()
        self.clients(online=[s1, s2], offline=[s3])
