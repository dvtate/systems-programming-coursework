CC      = gcc
CFLAGS  = -g -Wall -std=c99

all: mytr myzip myunzip

mytr myzip myunzip: % : %.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJS) mytr myzip myunzip
	rm -rf tests-out

test: all
	./run-tests.sh
