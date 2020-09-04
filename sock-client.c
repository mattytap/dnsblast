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
#define PORT 2300
int main()
{
    char buffer[MAXRCVLEN + 1];
    int sockfd, datasize;
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(PORT);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    if ((connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0))
    {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }
    //
    datasize = recv(sockfd, buffer, MAXRCVLEN, 0);
    buffer[datasize] = '\0';
    printf("Received from server: '%s' (%d bytes)\n", buffer, datasize);
    close(sockfd);
    return EXIT_SUCCESS;
}
