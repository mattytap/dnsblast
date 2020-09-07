// https://www.geeksforgeeks.org/udp-server-client-implementation-c/
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
#define PORT 2300
#define MAXLINE 1024
#define SOCK_TYPE SOCK_DGRAM
#define INTERNET_PROTOCOL 0

int main()
{
    const char *msg = "Welcome to server";
    int sockfd, data_sockfd;
    char buffer[MAXLINE]; // only needed for UDP?
    struct sockaddr_in servaddr;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
        if ((sockfd = socket(AF_INET, SOCK_TYPE, INTERNET_PROTOCOL)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&clientaddr, 0, sizeof(clientaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    /*
    BIND
    ------
    RECVFROM
    SENDTO
    */
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    do
    {
        // RECVFROM/SENDTO
        data_sockfd = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&clientaddr, &clientlen);
        buffer[data_sockfd] = '\0'; // only needed for UDP?
        printf("Incoming connection from %s - sending a welcome msg\n", inet_ntoa(clientaddr.sin_addr));
        printf("Client buffer: %s", buffer); // only needed for UDP?
        // SENDTO/RECVFROM
        sendto(sockfd, msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *)&clientaddr, clientlen);
        printf("Hello message sent: '%s' (%ld bytes)\n", msg, strlen(msg));
        close(data_sockfd);
    } while (data_sockfd);
    close(sockfd);
    return EXIT_SUCCESS;
}
