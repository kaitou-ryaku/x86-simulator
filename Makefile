CC = gcc
CFLAGS = -std=c99 -O0 -Wall

TARGET = $(notdir $(PWD)).out
ALL_C  = $(wildcard src/*.c)
ALL_CH = $(wildcard src/*.c include/*.h)
ALL_O  = $(patsubst src/%.c,object/%.o,$(ALL_C))

$(TARGET): $(ALL_CH)
	cd object && $(MAKE) "CC=$(CC)" "CFLAGS=$(CFLAGS)"
	$(CC) $(CFLAGS) $(ALL_O) -o $@

.PHONY: clean
clean:
	@rm -rf *.out *.o *.bin *.stackdump tmp* object/*.o object/*.d
