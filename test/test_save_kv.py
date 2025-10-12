from methods import TestDB

def run_tests(t):
    # insert elemets, save, verify elements in savefile [data.safe]
    for i in range(20):
        key = f"key-{i}"
        value = f"value-{i}"
        str_indb = f"{key}:{value}"
        cmd = f"SET {key} {value}"
        expected_str = t.expected_list(key, value)
        ret = t.exec_cmd(cmd, expected_str)
        expected = 0
        if ret != expected:
            print(f"Failed for key => {key}, value => {value}")
        # or else it worked
        # exec save cmd
        cmd = "SAVE"
        expected = 2
        ret = t.exec_cmd(cmd, expected_str)
        if ret != expected:
            print("SAVE failed!!")
        # verify content in file
        in_file = t.search_save_file(str_indb)
        if not in_file:
            print(f"Data corresponding to {key} {value} not found in savefile!!")
        else:
            print(f"Data corresponding to {key} {value} found in savefile")

if __name__ == '__main__':
    test = TestDB()
    run_tests(test)
    test._exit()