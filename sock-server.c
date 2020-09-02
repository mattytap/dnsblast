// https://en.wikibooks.org/wiki/C_Programming/Networking_in_UNIX
// does not die

//#define _POSIX_C_SOURCE 200112L

#include <stdio.h>	//printf
#include <stdlib.h> //for exit(0);
#include <string.h> //memset
#include <errno.h> //For errno - the error number#include <unistd.h> 
#include <netdb.h> //hostent
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
 
#define PORTNUM 2300
 
int main(int argc, char *argv[])
{
    char* msg = "Hello World !\n";
  
    struct sockaddr_in dest; /* socket info about the machine connecting to us */
    struct sockaddr_in serv; /* socket info about our server */
    int mysocket;            /* socket used to listen for incoming connections */
    socklen_t socksize = sizeof(struct sockaddr_in);

    memset(&serv, 0, sizeof(serv));           /* zero the struct before filling the fields */
    serv.sin_family = AF_INET;                /* set the type of connection to TCP/IP */
    serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
    serv.sin_port = htons(PORTNUM);           /* set the server port number */    

    mysocket = socket(AF_INET, SOCK_STREAM, 0);
  
    /* bind serv information to mysocket */
    bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));

    /* start listening, allowing a queue of up to 1 pending connection */
    listen(mysocket, 1);
    int consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
  
    while(consocket)
    {
        printf("Incoming connection from %s - sending welcome\n", inet_ntoa(dest.sin_addr));
        send(consocket, msg, strlen(msg), 0); 
        close(consocket);
        consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
    }

    close(mysocket);
    return EXIT_SUCCESS;
}
