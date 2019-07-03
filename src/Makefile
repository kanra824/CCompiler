CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mdcc: $(OBJS)
	$(CC) -o mdcc $(OBJS) $(LDFLAGS)

$(OBJS): mdcc.h

test: mdcc
	./test.sh

clean:
	rm -f mdcc *.o *~ tmp tmp.s

.PHONY: test clean