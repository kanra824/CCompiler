CFLAGS=-std=c11 -g -static
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
	rm -f mdcc *.o *~ tmp tmp.s a.out output.txt

.PHONY: test clean