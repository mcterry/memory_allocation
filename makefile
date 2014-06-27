TARGETS=test_smallalloc 

all:$(TARGETS)

test_smallalloc:test_smallalloc.c smallalloc.c smallalloc.h
	gcc -o $@ test_smallalloc.c smallalloc.c -g -Wall -lpthread

clean:
	rm -f $(TARGETS)
