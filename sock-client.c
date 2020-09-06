// https://en.wikibooks.org/wiki/C_Programming/Networking_in_UNIX
//#define _POSIX_C_SOURCE 200112L
#include <stdio.h>  //printf, perror
#include <stdlib.h> //for exit(0);
#include <string.h> //memset
#include <errno.h>  //For errno - the error number
#include <unistd.h>
#include <netdb.h> //hostent
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h> //sockaddr_in
#include <sys/socket.h>
#define MAXRCVLEN 500
#define HOST (in_addr_t) inet_addr("127.0.0.1")
#define PORT 2300
#define MAXLINE 1024
#define SOCK_TYPE SOCK_STREAM
#define INTERNET_PROTOCOL 0

int main()
{
    char msg[MAXRCVLEN + 1];
    int sockfd, msg_size;
    struct sockaddr_in servaddr;
    if ((sockfd = socket(AF_INET, SOCK_TYPE, INTERNET_PROTOCOL)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(PORT);
    /*

    CONNECT

    RECEIVE
    */
    // LISTEN/CONNECT
    // LISTEN/CONNECT
    if ((connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0))
    {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }
    // SEND/RECV
    msg_size = recv(sockfd, msg, MAXRCVLEN, 0);
    msg[msg_size] = '\0';
    printf("Received from server: '%s' (%d bytes)\n", msg, msg_size);
    close(sockfd);
    return EXIT_SUCCESS;
}
