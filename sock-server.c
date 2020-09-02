// https://en.wikibooks.org/wiki/C_Programming/Networking_in_UNIX
// does not die
//#define _POSIX_C_SOURCE 200112L
#include <stdio.h>  //printf
#include <stdlib.h> //for exit(0);
#include <string.h> //memset
#include <errno.h>  //For errno - the error number
#include <unistd.h>
#include <netdb.h> //hostent
// #include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h> //sockaddr_in
#include <sys/socket.h>
#define PORT 2300
// int main(int argc, char *argv[])
int main()
{
    char *msg = "Hello World !\n";
    int mysocket, datasize;        /* socket used to listen for incoming connections */
    struct sockaddr_in clientaddr; /* socket info about the machine connecting to us */
    struct sockaddr_in servaddr; /* socket info about our server */
    socklen_t socksize = sizeof(struct sockaddr_in);
    memset(&servaddr, 0, sizeof(servaddr));            /* zero the struct before filling the fields */
    servaddr.sin_family = AF_INET;                     /* set the type of connection to TCP/IP */
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);      /* set our address to any interface */
    servaddr.sin_port = htons(PORT);                   /* set the server port number */
    mysocket = socket(AF_INET, SOCK_STREAM, 0);
    bind(mysocket, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)); // bind serv information to mysocket
    listen(mysocket, 1); // start listening, allowing a queue of up to 1 pending connection
    datasize = accept(mysocket, (struct sockaddr *)&clientaddr, &socksize); // data exchanged client->server
    while (datasize)
    {
        printf("Incoming connection from %s - sending welcome\n", inet_ntoa(clientaddr.sin_addr));
        send(datasize, msg, strlen(msg), 0);
        close(datasize);
        datasize = accept(mysocket, (struct sockaddr *)&clientaddr, &socksize);
    }
    close(mysocket);
    return EXIT_SUCCESS;
}
