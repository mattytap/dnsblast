// traditional gethostbyname function to retrieve information about a hostname/domain name.
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
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
	// pertinent function
	if ((he = gethostbyname(hostname)) == NULL)
	{
		// get the host info
		perror("gethostbyname");
		return 1;
	}
	addr_list = (struct in_addr **)he->h_addr_list;
	for (i = 0; addr_list[i] != NULL; i++)
	{
		//Return the first one;
		strcpy(ip, inet_ntoa(*addr_list[i]));
		return 0;
	}
	return 1;
}