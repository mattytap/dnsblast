
OPTIMIZATION ?= -O2
STDFLAGS ?= -std=c99
DEBUGFLAGS ?= -Waggregate-return -Wcast-align -Wcast-qual \
-Wchar-subscripts -Wcomment -Wimplicit -Wmissing-declarations \
-Wmissing-prototypes -Wnested-externs -Wparentheses -Wwrite-strings \
-Wformat=2 -Wall -Wextra

CFLAGS ?= $(OPTIMIZATION) $(STDFLAGS) $(DEBUGFLAGS)

all: dnsblast.o mattblast sock-client sock-server ClientTCP getaddrinfo gethostbyname UDP-client UDP-server 

mattblast: Makefile mattblast.o
	$(CC) mattblast.o -o mattblast $(LDFLAGS)

sock-client: Makefile sock-client.o
	$(CC) sock-client.o -o sock-client $(LDFLAGS)

sock-server: Makefile sock-server.o
	$(CC) sock-server.o -o sock-server $(LDFLAGS)

ClientTCP: Makefile ClientTCP.o
	$(CC) ClientTCP.o -o ClientTCP $(LDFLAGS)

getaddrinfo: Makefile getaddrinfo.o
	$(CC) getaddrinfo.o -o getaddrinfo $(LDFLAGS)

gethostbyname: Makefile gethostbyname.o
	$(CC) gethostbyname.o -o gethostbyname $(LDFLAGS)

UDP-client: Makefile UDP-client.o
	$(CC) UDP-client.o -o UDP-client $(LDFLAGS)

UDP-server: Makefile UDP-server.o
	$(CC) UDP-server.o -o UDP-server $(LDFLAGS)


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
