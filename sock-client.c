// https://en.wikibooks.org/wiki/C_Programming/Networking_in_UNIX
//#define _POSIX_C_SOURCE 200112L
#include <stdio.h>  //printf
#include <stdlib.h> //for exit(0);
#include <string.h> //memset
#include <errno.h>  //For errno - the error number
#include <unistd.h>
#include <netdb.h> //hostent
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h> //sockaddr_in
#include <sys/socket.h>
#define MAXRCVLEN 500
#define PORT 2300
// int main(int argc, char *argv[])
int main()
{
    char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
    int mysocket, datasize;        /* socket used to listen for incoming connections */
    struct sockaddr_in servaddr; /* socket info about our server */
    socklen_t socksize = sizeof(struct sockaddr_in);
    memset(&servaddr, 0, sizeof(servaddr));            /* zero the struct before filling the fields */
    servaddr.sin_family = AF_INET;                     /* set the type of connection to TCP/IP */
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* set destination IP number - localhost, 127.0.0.1*/
    servaddr.sin_port = htons(PORT);                   /* set the server port number */
    mysocket = socket(AF_INET, SOCK_STREAM, 0);
    // connect to server on mysocket
    connect(mysocket, (struct sockaddr *)&servaddr, socksize);
    datasize = recv(mysocket, buffer, MAXRCVLEN, 0); // data exchanged client->server
    buffer[datasize] = '\0'; // We have to null terminate the received data ourselves
    while (datasize)
    {
        printf("Received %s (%d bytes).\n", buffer, datasize);
        datasize = 0;
    }
    close(mysocket);
    return EXIT_SUCCESS;
}
