import unittest
import pexpect
import time, io, sys, os, re
from itertools import chain

# Settings
# --------

def localdir(location):
    return os.path.join(os.path.dirname(__file__), location)

click_loc = localdir("../../click/userlevel/click")


# Color stuff
# -----------

endc = '\033[0m'  # Resets all ANSI attributes
red = (255, 0, 0)
green = (0, 255, 0)
blue = (0, 0, 255)
cyan = (0, 255, 255)
pink = (255, 0, 255)
yellow = (255, 255, 0)
black = (0, 0, 0)
white = (255, 255, 255)
 
def _rgb(rf,gf,bf,rb=0,gb=0,bb=0):
    # red front, ..., blue back
    return "\033[1;38;2;{};{};{};48;2;{};{};{}m".format(rf,gf,bf,rb,gb,bb)

def rgb(f, b=black):
    return _rgb(f[0], f[1], f[2], b[0], b[1], b[2])

def rgbtext(s, f=red, b=black):
    return rgb(f,b) + s + endc

class ColoredWrite:
    def __init__(self, f):
        self.f = f
    
    def write(self, b: bytes):
        self.f.write(rgbtext("--- " + b.decode("utf-8")).encode())
        
    def __getattr__(self, attr):
        return getattr(self.f, attr)


# pexpect hacks
# -------------

def not_expect(self, *a, **kw):
    try:
        self.expect(*a, **kw)
    except pexpect.exceptions.TIMEOUT:
        return
    raise AssertionError("Should not have found expression " + str(a))

pexpect.spawn.not_expect = not_expect

ip_re = "\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}"

# Base class
# ----------

class ClickTest(unittest.TestCase):
    def setUp(self):
        self.click_buffer = io.BytesIO()
        self.click = pexpect.spawn(click_loc + " " + self.script + " -p 5000", 
                                   logfile=self.click_buffer, timeout=2)
        time.sleep(0.5)
        self.telnet = pexpect.spawn("telnet 127.0.0.1 5000")
        self.telnet.logfile_send = ColoredWrite(self.click_buffer)
        self.telnet.expect("ControlSocket")
    
    def tearDown(self):
        if hasattr(self, '_outcome'):  # Python 3.4+
            result = self.defaultTestResult()  # these 2 methods have no side effects
            self._feedErrorsToResult(result, self._outcome.errors)
        else:  # Python 3.2 - 3.3 or 2.7
            result = getattr(self, '_outcomeForDoCleanups', self._resultForDoCleanups)
        error = self.list2reason(result.errors)
        failure = self.list2reason(result.failures)
        ok = not error and not failure

        if not ok:
            print("Something went wrong, printing the log:")
            print(self.click_buffer.getvalue().decode("utf-8"))

    def list2reason(self, exc_list):
        if exc_list and exc_list[-1][0] is self:
            return exc_list[-1][1]
    
    def clients(self, *, online=[], offline=[], group=None, source=None, **kw):
        both = online + offline
        if group is None: group = ip_re
        else: group = group.replace(".", "\.")
        if source is None: source = ip_re
        else: source = source.replace(".", "\.")
        extra_message = (" for group " + group.replace("\.", ".") if group != ip_re else "") +\
                        (" for source " + source.replace("\.", ".") if source != ip_re else "")
        
        patterns = ["{} -- received a packet.*{source}.*{group}".format(i, group=group, source=source) for i in both]
        self.click_buffer.write(rgbtext("--- asserting online = " + str(online) + ", offline = " + str(offline) + extra_message + "\n", cyan).encode())
        index = self.click.expect(pexpect.TIMEOUT)
        before = self.click.before.decode("utf-8")
        for i, p in enumerate(patterns):
            self.assertEqual(i < len(online), re.search(p, before) is not None,
                             (both[i] + " should be " + "online" if i < len(online) else "offline") + extra_message)

