// Server side implementation of UDP client-server model
// dies
//#define _POSIX_C_SOURCE 200112L
#include <stdio.h>  //printf
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
// int main(int argc, char *argv[])
int main()
{
    const char *hello = "Hello from server\n";
    int sockfd, n; /* socket used to listen for incoming connections */
    char buffer[MAXLINE];
    struct sockaddr_in servaddr; /* socket info about our server */
    struct sockaddr_in cliaddr;
    memset(&servaddr, 0, sizeof(servaddr)); /* zero the struct before filling the fields */
    memset(&cliaddr, 0, sizeof(cliaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;                /* set the type of connection to TCP/IP */
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
    servaddr.sin_port = htons(PORT);              /* set the server port number */
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) //UDP use SOCK_DGRAM
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    int len;
    len = sizeof(cliaddr); //len is value/resuslt
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&cliaddr, len);
    buffer[n] = '\0';
    printf("Client : %s\n", buffer);
    sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
    printf("Hello message sent.\n");
    return 0;
}
