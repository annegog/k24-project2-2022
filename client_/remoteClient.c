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

    int port, sock, reading;
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
    printf("Connecting to %s with port %d\n\n", rem->h_name, port);

    // write the path we want to copy on client's file system
    if (write(sock, directory, strlen(directory)+1) < 0)
        perror_exit("write");

    // creating a folder to save the files we want
    char* dir_to_make = "./Client";
    mkdir(dir_to_make,S_IRWXU);

    char new_file[BUFF];

    while(1){

        read_data(sock,buffer);
        char* file = separate(buffer);
        printf("Received File: %s\n",file);
        
        /*********************************************************/
        
        // cheking if the file allready exists
        find_the_file(dir_to_make,file);
        
        sprintf(new_file, "%s/%s", dir_to_make, file);
        
        // create a file in the folder Client
        // so we copy the (server) file to client's file        
        FILE *write_file = fopen(new_file, "w+");
        if ( write_file == NULL){   
            printf("Error! Could not open file\n"); 
            exit(EXIT_FAILURE); 
        }
        // while( (reading = read(sock, buffer_read, BUFF)) > 0){
        //     printf("%s", buffer_read);
        //     fprintf(write_file,"%s", buffer_read);
        // }
  
        fprintf(write_file,"Hi! I'm %s :( ..", file);
        
        fclose(write_file);
        
    }

    return 0;
}
