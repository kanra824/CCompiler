CFLAGS=-std=c11 -g -O0 -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mdcc: $(OBJS)
	$(CC) -o mdcc $(OBJS) $(LDFLAGS)

$(OBJS): mdcc.h

test: mdcc
	./test.sh

debug: $(OBJS)
	$(CC) -g -O0 -o mdcc $(OBJS) $(LDFLAGS)

clean:
	rm -f mdcc *.o *~ tmp tmp.s a.out tmp.c

.PHONY: test clean