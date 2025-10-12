.DEFAULT_GOAL := build

build:
	$(MAKE) -C src

test:
	python ./test/insert_get_update.py
	rm -rf pexpect_debug.log
# 	python3 -m unittest discover -s test

clean:
	$(MAKE) -C src clean
	rm -rf build/* && rmdir build

all: build test clean

.PHONY: all build test clean