// Client side implementation of UDP client-server model

//#define _POSIX_C_SOURCE 200112L
#include <stdio.h>  //printf, perror
#include <stdlib.h> //for exit(0);
#include <string.h> //memset
// #include <errno.h> //For errno - the error number
#include <unistd.h>
// #include <netdb.h> //hostent
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h> //sockaddr_in
#include <sys/socket.h>
#define MAXLINE 1024
#define HOST (in_addr_t) inet_addr("127.0.0.1")
#define PORT 2300
int main()
{
    char buffer[MAXLINE];
    const char *hello = "Hello from client\n";
    int sockfd, datasize;
    struct sockaddr_in servaddr;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // zap server information
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;   // IP protocol family.
    servaddr.sin_port = htons(PORT); // 53 Big Endian short interger
    servaddr.sin_addr.s_addr = INADDR_ANY; // Big Endian format. see htonl()
    
    int len;
    sendto(sockfd, (const char *)hello, strlen(hello), 
    MSG_CONFIRM, (const struct sockaddr *)&servaddr, 
    sizeof(servaddr));
    printf("Hello message sent.\n");

    datasize = recvfrom(sockfd, (char *)buffer, MAXLINE, 
    MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    buffer[datasize] = '\0';
    printf("Received from server: '%s' (%d bytes)\n", buffer, datasize);

    close(sockfd);
    return 0;
}
