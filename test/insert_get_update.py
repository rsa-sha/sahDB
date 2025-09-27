import random
from methods import TestDB


def run_tests(t):
    # 100 inserts test
    for i in range(1, 100):
        key = f"key_{i}"
        value = f"value_{i}"
        cmd = f"SET {key} {value}"
        expected_str = f"Added key {key} and value {value} in DB"
        ret = t.exec_cmd(cmd, expected_str)
        if not ret:
            print(f"Failed for key =>{key}, value => {value}")



if __name__ == '__main__':
    test = TestDB()
    run_tests(test)
    test._exit()