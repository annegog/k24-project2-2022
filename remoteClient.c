// Client
#include <stdio.h>
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <unistd.h>          /* read, write, close */
#include <netdb.h>	         /* gethostbyaddr */
#include <stdlib.h>	         /* exit */
#include <string.h>	         /* strlen */

#define BUFF 4096

void perror_exit(char *message);


main(int argc, char *argv[]){

    int port, sock, i;
    char buf[BUFF];
    char *directory;

    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *rem;
    
    // Create socket
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    	perror_exit("socket");

    if (argc != 7){
        // ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>
        printf("Worng arguments\n");
        exit(1);
    }
    printf("Client’s parameters are:\n");
    for (int i = 0; i < argc; i++){
        if (strcmp(argv[i], "-i") == 0){
            // Find server address
            if ((rem = gethostbyname(argv[i+1])) == NULL) {	
	        herror("gethostbyname"); exit(1);
            }
            printf("serverIP: %s\n", rem->h_name);
        }
        if (strcmp(argv[i], "-p") == 0){
            port = atoi(argv[i + 1]);
            printf("port: %d\n", port);
        }
        if (strcmp(argv[i], "-d") == 0){
            directory = argv[i + 1];
            printf("directory: %s\n", directory);
        }
    }

    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);         /* Server port */
    /* Initiate connection */
    if (connect(sock, serverptr, sizeof(server)) < 0)
	   perror_exit("connect");
    printf("Connecting to %s port %d\n", rem->h_name, port);

    strcpy(buf,directory);
    while (1){
    
        // write the path we want to copy on client's file system
        if (write(sock,buf, BUFF) < 0)
            perror_exit("write");
    

    }



}


void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}