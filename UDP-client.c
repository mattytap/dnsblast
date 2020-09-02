// Client side implementation of UDP client-server model

//#define _POSIX_C_SOURCE 200112L
#include <stdio.h>	//printf
#include <stdlib.h> //for exit(0);
#include <string.h> //memset
// #include <errno.h> //For errno - the error number
#include <unistd.h> 
// #include <netdb.h> //hostent
#include <unistd.h>
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <netinet/in.h>
#include <sys/socket.h>

#define HOST    ((const unsigned char *)"127.0.0.1")
#define PORT 2300
#define MAXLINE 1024
// Driver code 
int main()
{ 
    int sockfd;
    char buffer[MAXLINE];
    char *hello = "Hello from client"; 
    struct sockaddr_in     servaddr; 
  
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // zap server information
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IP protocol family.
    servaddr.sin_port = htons(PORT); // 53 Big Endian short interger
    servaddr.sin_addr.s_addr = inet_addr(HOST); // Big Endian format. see htonl()
//    const char *matt = inet_ntop(AF_INET,0,&servaddr.sin_addr.s_addr,strlen(&servaddr.sin_addr.s_addr));

//    printf("Matt: %s\n", matt);
//    int n, len;
//    size_t sendtov;
//    if ((sendtov = sendto(sockfd, (const char *)hello, strlen(hello), 
//        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
//            sizeof(servaddr))) != strlen(hello))
//    {
//        perror("sendto failed");
//        exit(EXIT_FAILURE);
//    }
    int n, len;
    sendto(sockfd, (const char *)hello, strlen(hello), 
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
    printf("Hello message sent.\n");

    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &servaddr, 
                &len);
    buffer[n] = '\0';
    printf("Server : %s\n", buffer);

    close(sockfd);
    return 0;
}
