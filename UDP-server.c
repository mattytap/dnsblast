// https://www.geeksforgeeks.org/udp-server-client-implementation-c/
//#define _POSIX_C_SOURCE 200112L
#include <stdio.h>  //printf, perror
#include <stdlib.h> //for exit(0);
#include <string.h> //memset
// #include <errno.h> //For errno - the error number
#include <unistd.h>
//#include <netdb.h> //hostent
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h> //sockaddr_in
#include <sys/socket.h>

#define PORT 2300
#define MAXLINE 1024
int main()
{
    const char *msg = "Welcome to server";
    int sockfd, datasize;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;
    socklen_t socksize = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&clientaddr, 0, sizeof(clientaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    //
    datasize = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&clientaddr, &socksize);
    buffer[datasize] = '\0';
    while (datasize)
    {
        printf("Client : %s\n", buffer);
        sendto(sockfd, msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *)&clientaddr, socksize);
        printf("Hello message sent.\n");
        close(datasize);
        datasize = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&clientaddr, &socksize);
    }
    close(sockfd);
    return EXIT_SUCCESS;
}
