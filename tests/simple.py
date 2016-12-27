
import unittest
import time

from base import *

    
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

