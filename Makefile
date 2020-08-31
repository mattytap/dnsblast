
OPTIMIZATION ?= -O2
STDFLAGS ?= -std=c99
DEBUGFLAGS ?= -Waggregate-return -Wcast-align -Wcast-qual \
-Wchar-subscripts -Wcomment -Wimplicit -Wmissing-declarations \
-Wmissing-prototypes -Wnested-externs -Wparentheses -Wwrite-strings \
-Wformat=2 -Wall -Wextra

CFLAGS ?= $(OPTIMIZATION) $(STDFLAGS) $(DEBUGFLAGS)

all: ClientTCP matt mattnew dnsblast.o

mattnew: Makefile mattnew.o
	$(CC) mattnew.o -o mattnew $(LDFLAGS)

matt: Makefile matt.o
	$(CC) matt.o -o matt $(LDFLAGS)

ClientTCP: Makefile ClientTCP.o
	$(CC) ClientTCP.o -o ClientTCP $(LDFLAGS)

dnsblast.o: Makefile dnsblast.c dns.h dnsblast.h
	$(CC) -c dnsblast.c -o dnsblast.o $(CFLAGS)

clean:
	rm -f dnsblast *.a *.d *.o
	rm -f mattnew *.a *.d *.o
	rm -f matt *.a *.d *.o
	rm -f ClientTCP *.a *.d *.o
	rm -rf *.dSYM
