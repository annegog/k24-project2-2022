// Client
#include <stdio.h>
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <unistd.h>          /* read, write, close */
#include <netdb.h>	         /* gethostbyaddr */
#include <stdlib.h>	         /* exit */
#include <string.h>	         /* strlen */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "client.h"

int main(int argc, char *argv[]){

    int port, sock;
    char buffer[BUFSIZ];
    char buffer_read[BUFSIZ];
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
    printf("Connecting to %s port %d\n\n", rem->h_name, port);

    // write the path we want to copy on client's file system
    if (write(sock, directory, strlen(directory)+1) < 0)
        perror_exit("write");

    while(1){

        read_data(sock,buffer);
        
        char* file = separate(buffer);
        printf("File: %s\n",file);
        
        /*********************************************************/
        int fd = find_the_file(directory,file);

        // create a file in the current dir.
        // so we copy the (server) file to the /file        
        FILE *write_file = fopen(file, "w");
        if ( write_file == NULL){   
            printf("Error! Could not open file\n"); 
            exit(EXIT_FAILURE); 
        }

        // while( recv(sock, buffer_read, BUFF, 0) > 0){
        //     //printf(">>THE DATA:\n%s //telos\n", buffer_read);
        //     fprintf(write_file,"%s", buffer_read);
        //     sleep(1);
        // }
        // usleep(10000);
        // printf("out of the while recv\n");

        fprintf(write_file,"skata...out of the while!");
        
        fclose(write_file);
        close(fd);
    }

    return 0;
}