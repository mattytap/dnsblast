
OPTIMIZATION ?= -O2
STDFLAGS ?= -std=c99
DEBUGFLAGS ?= -Waggregate-return -Wcast-align -Wcast-qual \
-Wchar-subscripts -Wcomment -Wimplicit -Wmissing-declarations \
-Wmissing-prototypes -Wnested-externs -Wparentheses -Wwrite-strings \
-Wformat=2 -Wall -Wextra

CFLAGS ?= $(OPTIMIZATION) $(STDFLAGS) $(DEBUGFLAGS)

all: matt mattnew ClientTCP Untitled-server Untitled-client dns5 dns6 server dnsblast.o

mattnew: Makefile mattnew.o
	$(CC) mattnew.o -o mattnew $(LDFLAGS)

matt: Makefile matt.o
	$(CC) matt.o -o matt $(LDFLAGS)

ClientTCP: Makefile ClientTCP.o
	$(CC) ClientTCP.o -o ClientTCP $(LDFLAGS)

Untitled-server: Makefile Untitled-server.o
	$(CC) Untitled-server.o -o Untitled-server $(LDFLAGS)

Untitled-client: Makefile Untitled-client.o
	$(CC) Untitled-client.o -o Untitled-client $(LDFLAGS)

dns5: Makefile dns5.o
	$(CC) dns5.o -o dns5 $(LDFLAGS)

dns6: Makefile dns6.o
	$(CC) dns6.o -o dns6 $(LDFLAGS)

server: Makefile server.o
	$(CC) -c server.o -o server $(CFLAGS)



dnsblast.o: Makefile dnsblast.c dns.h dnsblast.h
	$(CC) -c dnsblast.c -o dnsblast.o $(CFLAGS)



clean:
	rm -f dnsblast *.a *.d *.o
	rm -f mattnew *.a *.d *.o
	rm -f matt *.a *.d *.o
	rm -f Untitled-server *.a *.d *.o
	rm -f Untitled-client *.a *.d *.o
	rm -f dns5 *.a *.d *.o
	rm -f dns6 *.a *.d *.o
	rm -f ClientTCP *.a *.d *.o
	rm -f server
	rm -rf *.dSYM
