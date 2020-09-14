// Client side implementation of UDP client-server model
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

#define HOST (in_addr_t) inet_addr("127.0.0.1")
#define PORT 2300
#define MAXLINE 1024
#define SOCK_TYPE SOCK_DGRAM
#define INTERNET_PROTOCOL 0

int main()
{
    const char *msg = "Hello from client\n";
    int sockfd, datasize;
    char buffer[MAXLINE]; // only needed for UDP?
    struct sockaddr_in servaddr;
    if ((sockfd = socket(AF_INET, SOCK_TYPE, INTERNET_PROTOCOL)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    /*
    -----
    ------
    SENDTO.
    RECVFROM.
    */

    socklen_t len;
    // RECVFROM/SENDTO
    sendto(sockfd, (const char *)msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("Hello message sent.\n");
    // SENDTO/RECVFROM
    datasize = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    buffer[datasize] = '\0';
    printf("Received from server: '%s' (%d bytes)\n", buffer, datasize);

    close(sockfd);
    return 0;
}
