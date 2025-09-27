import pexpect
import logging
import time

BIN = '../src/sahDB'

class TestDB:
    def __init__(self):
        self.bin = self.__spawn_bin()
        self.bin.logfile = open("pexpect_debug.log", "w")


    def exec_cmd(self, cmd, expect) -> bool:
        idx = self.bin.expect('sahDB>')
        self.bin.sendline(cmd)
        # time.sleep(0.05)
        idx = self.bin.expect([expect], timeout = 5)
        if idx == 0:
            return True
        else:
            return False


    def __spawn_bin(self):
        """Start executable"""
        try:
            proc = pexpect.spawn(BIN, encoding='utf-8', timeout=10)
        except:
            print(f"Failed to start process: {e}")
            return None
        else:
            return proc
    
    def _exit(self):
        self.bin.sendline('EXIT')
