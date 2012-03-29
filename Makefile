
RM=/bin/rm -f

CC=gcc
CFLAGS=-Wall -pipe -s

wakeup: wakeup.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	$(RM) wakeup

.PHONY: distclean

distclean: clean
