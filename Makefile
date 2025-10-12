.DEFAULT_GOAL := build

build:
	$(MAKE) -C src

# test:
# 	python ./test/test_*.py
# 	rm -rf pexpect_debug.log
# # 	python3 -m unittest discover -s test
test:
	for test_file in ./test/test_*.py; do \
		echo "Running $$test_file..."; \
		python $$test_file > /dev/null 2>&1; \
		exit_code=$$?; \
		echo "$$test_file ran with exit code $$exit_code"; \
		rm -rf pexpect_debug.log; \
	done

clean:
	$(MAKE) -C src clean
	rm -rf build/* && rmdir build

all: build test clean

.PHONY: all build test clean
