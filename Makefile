BUILD_DIR := build
GO_DIR := network/go
SHELL := /bin/bash

all: build

build:
	@$(MAKE) -C src
	@mkdir -p $(BUILD_DIR)
	@mv src/libkv.a $(BUILD_DIR)/
	@$(MAKE) -C network/c
	@mv network/c/sahDB $(BUILD_DIR)/

build-go:
	@cd $(GO_DIR) && CGO_ENABLED=1 go build -o $(abspath $(BUILD_DIR))/sahDB_go
	@echo "Go binary built successfully at $(BUILD_DIR)/sahDB_go"


PYTHON := /usr/bin/python3  # Set the correct Python path here

test:
	@echo "Running Unit tests..."
	@TESTS=$(wildcard test/test_*.py); \
	for test in $$TESTS; do
		echo "Running $$test..."; \
		$(PYTHON) $$test
	done

clean:
	@$(MAKE) -C src clean
	@$(MAKE) -C network/c clean
	@$(MAKE) -C tools clean || true
# 	@$(MAKE) -C persistence clean || true
	@rm -rf $(BUILD_DIR)
	@rm -rf {pexpect_debug.log,test/{*pyc*}}
	@rm -fr cppcheck*

cleanlogs:
	@rm -rf {pexpect_debug.log,test/{*pyc*}}
	@rm -fr cppcheck*


check:
	cppcheck --enable=all --inconclusive --std=c11 --platform=unix64 \
		--suppress=missingIncludeSystem --force -I include/ --xml --xml-version=2 . 2> cppcheck.xml
	cppcheck-htmlreport --file=cppcheck.xml --report-dir=cppcheck-html
	xdg-open cppcheck-html/index.html



.PHONY: all build build-go clean test check