import os
import pexpect
import logging
import time

# for toplevel makefile
BIN = 'build/sahDB'
SAVE_FILE = "data.safe"
# for exec from cur-dir comment prev line, uncomment next line
# BIN = '../build/sahDB'

class TestDB:
    def __init__(self):
        self.bin = self.__spawn_bin()
        self.bin.logfile = open("pexpect_debug.log", "w")
        self.ex_dict = {
            "IN_OK"     : "Added key {key} and value {value} in DB",
            "DB_FULL"   : "DB storage is full",
            "SAVE"      : f"Data from disk written to {SAVE_FILE}"
        }

    def expected_list(self, key = "", value = ""):
        data = [
            self.ex_dict['IN_OK'].format(key = key, value = value),
            self.ex_dict['DB_FULL'],
            self.ex_dict['SAVE']
        ]
        return data

    def exec_cmd(self, cmd, expect) -> bool:
        idx = self.bin.expect('sahDB>')
        self.bin.sendline(cmd)
        idx = self.bin.expect(expect, timeout = 5)
        return idx

    def search_save_file(self, string) -> bool:
        data = []
        with open(SAVE_FILE, 'r') as f:
            data = f.readlines()
        for l in data:
            if string in l:
                return True
        return False

    def __spawn_bin(self):
        """Start executable"""
        try:
            proc = pexpect.spawn(BIN, encoding='utf-8', timeout=10)
        except Expection as e:
            print(f"Failed to start process: {e}")
            return None
        else:
            return proc
    
    def _exit(self):
        # cleaning up save file
        if os.path.exists(SAVE_FILE):
            os.remove(SAVE_FILE)
        self.bin.sendline('EXIT')
