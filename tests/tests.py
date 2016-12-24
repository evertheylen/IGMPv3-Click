
import unittest

from base import *


# Actual tests
# ------------
    
class Simple(ClickTest):
    script = localdir("../scripts/ipnetwork.click")
    
    def testSingleExclude(self):
        self.telnet.write("write client31/igmp.change_mode EXCLUDE 224.4.4.4\n")
        self.clients(online=["client31"], offline=["client32"])

if __name__ == '__main__':
    unittest.main()
