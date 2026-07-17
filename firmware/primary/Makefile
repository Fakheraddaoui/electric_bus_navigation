# Host-side unit tests with vendored Unity (no external tooling required).
# Usage: make test
CC      ?= gcc
CFLAGS  ?= -std=c99 -Wall -Wextra -g -O0
BUILD   := build
UNITY   := vendor/unity

# All library sources except RTOS entry points (main_*.c needs tx_api.h)
SRCS      := $(filter-out src/main_%.c,$(wildcard src/*.c))
TESTS     := $(wildcard test/test_*.c)
RUNNERS   := $(patsubst test/%.c,$(BUILD)/%_runner.c,$(TESTS))
BINS      := $(patsubst test/%.c,$(BUILD)/%,$(TESTS))
INCLUDES  := -Iinclude -I$(UNITY)

.PHONY: test clean
test: $(BINS)
	@rc=0; for b in $(BINS); do echo "---- $$b ----"; ./$$b || rc=1; done; exit $$rc

$(BUILD)/%_runner.c: test/%.c | $(BUILD)
	python3 tools/gen_runner.py $< $@

$(BUILD)/%: test/%.c $(BUILD)/%_runner.c $(SRCS) $(UNITY)/unity.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ -lm

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
