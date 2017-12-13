CPP  := $(CC) $(CFLAGS) -E
ALL_C = $(wildcard ../src/*.c)
ALL_H = $(wildcard ../include/*.h)
ALL_O = $(patsubst ../src/%.c,%.o,$(ALL_C))
ALL_D = $(patsubst ../src/%.c,%.d,$(ALL_C))

.PHONY: dummy
dummy: $(ALL_O)

%.o: ../src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.d: ../src/%.c $(ALL_H)
	$(CPP) -MM $< -MF $@

-include $(ALL_D)
