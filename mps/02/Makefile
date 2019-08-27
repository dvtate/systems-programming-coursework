CC      = gcc
CFLAGS  = -g -Wall

all: mytr myzip myunzip

mytr myzip myunzip: % : %.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJS) mytr myzip myunzip
	rm -rf tests-out

test: all
	./run-tests.sh
