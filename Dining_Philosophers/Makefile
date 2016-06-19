CFLAGS = -g -Wall 
LDFLAGS = -pthread

dp_harness: dp.o
	gcc $(CFLAGS) dp.o dp_harness.c -o dp_harness $(LDFLAGS)

.PHONY: clean
clean:
	rm -f dp_harness *.o
