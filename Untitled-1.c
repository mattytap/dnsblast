// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//#include <sys/types.h>
#include <sys/socket.h>
//#include <arpa/inet.h>
#include <netinet/in.h>
#define V4IP 0x0A000004
#define PORT 5335
#define MAXLINE 1024

int main()
{
    int sockfd;
    char buffer[MAXLINE];
    const char *hello = "Hello from client";
    struct sockaddr_in servaddr;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));  // zap server information
    // Filling server information
    servaddr.sin_family = AF_INET; // IP protocol family.
    servaddr.sin_port = htons(PORT); // 53
    servaddr.sin_addr.s_addr = INADDR_ANY; // (in_addr_t) 0x00000000
    servaddr.sin_addr.s_addr = 0x0A000004;
    int n, len;
    size_t sendtov;
    if ((sendtov = sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != strlen(hello))
    {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }
    printf("Hello message sent.\n");
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    buffer[n] = '\0';
    printf("Server : %s\n", buffer);
    close(sockfd);
    return 0;
}
