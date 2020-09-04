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
#define PORT 2300
int main()
{
    const char *msg = "Welcome to server";
    int sockfd, datasize;
    struct sockaddr_in servaddr;
    socklen_t socksize = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0))
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    listen(sockfd, 1); // 1 pending connection
    datasize = accept(sockfd, (struct sockaddr *)&clientaddr, &socksize);
    while (datasize)
    {
        printf("Incoming connection from %s - sending a welcome msg\n", inet_ntoa(clientaddr.sin_addr));
        send(datasize, msg, strlen(msg), 0);
        close(datasize);
        datasize = accept(sockfd, (struct sockaddr *)&clientaddr, &socksize);
    }
    close(sockfd);
    return EXIT_SUCCESS;
}
