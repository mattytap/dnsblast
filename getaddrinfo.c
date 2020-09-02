// getaddrinfo function to retrieve information about a hostname/domain name. The getaddrinfo supports ipv6 better.
// #define _POSIX_C_SOURCE 200112L
// stddef.h – Defines several useful types and macros.
// stdint.h – Defines exact width integer types.
#include <stdio.h>	//Defines core input and output functions eg printf
#include <stdlib.h> //Defines numeric conversion functions, pseudo-random network generator, memory allocation eg exit(0);
#include <string.h> //Defines string handling functions eg memset
// math.h – Defines common mathematical functions
#include <errno.h> //For errno - the error number
#include <netdb.h> //hostent
#include <arpa/inet.h>
#include <sys/socket.h>
int hostname_to_ip(char *, char *);
int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Please provide a hostname to resolve");
		exit(1);
	}
	char *hostname = argv[1];
	char ip[100];
	hostname_to_ip(hostname, ip);
	printf("%s resolved to %s", hostname, ip);
	printf("\n");
}
/*
	Get ip from domain name
 */
int hostname_to_ip(char *hostname, char *ip)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in *h;
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;
		// pertinent function
		if ((rv = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		h = (struct sockaddr_in *)p->ai_addr;
		strcpy(ip, inet_ntoa(h->sin_addr));
	}
	freeaddrinfo(servinfo); // all done with this structure
	return 0;
}