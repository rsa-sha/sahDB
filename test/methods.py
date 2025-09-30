import pexpect
import logging
import time

BIN = '../src/sahDB'

class TestDB:
    def __init__(self):
        self.bin = self.__spawn_bin()
        self.bin.logfile = open("pexpect_debug.log", "w")
        self.ex_dict = {
            "IN_OK": "Added key {key} and value {value} in DB",
            "DB_FULL" : "DB storage is full"
        }

    def expected_list(self, key = "", value = ""):
        data = [
            self.ex_dict['IN_OK'].format(key = key, value = value),
            self.ex_dict['DB_FULL']
        ]
        return data

    def exec_cmd(self, cmd, expect) -> bool:
        idx = self.bin.expect('sahDB>')
        self.bin.sendline(cmd)
        idx = self.bin.expect(expect, timeout = 5)
        return idx


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
