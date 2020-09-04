// Client side implementation of UDP client-server model

//#define _POSIX_C_SOURCE 200112L
#include <stdio.h>  //printf
#include <stdlib.h> //for exit(0);
#include <string.h> //memset
// #include <errno.h> //For errno - the error number
#include <unistd.h>
// #include <netdb.h> //hostent
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define HOST (in_addr_t) inet_addr("127.0.0.1")
#define PORT 2300
#define MAXLINE 1024
// int main(int argc, char *argv[])
int main()
{
    const char *hello = "Hello from client\n";
    int sockfd, n; /* socket used to listen for incoming connections */
    char buffer[MAXLINE];
    struct sockaddr_in servaddr; /* socket info about our server */

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) //UDP use SOCK_DGRAM
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // zap server information
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;   // IP protocol family.
    servaddr.sin_port = htons(PORT); // 53 Big Endian short interger
    servaddr.sin_addr.s_addr = HOST; // Big Endian format. see htonl()
    
    int n, len;
    sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Hello message sent.\n");

    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    buffer[n] = '\0';
    printf("Server : %s\n", buffer);

    close(sockfd);
    return 0;
}
