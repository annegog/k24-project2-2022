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


char* separate(char* path){
    char s[2] = "/";
    char* temp;
    char* file = path;
    temp = strtok(path, s);
    while( temp != NULL ){
        strcpy(file, temp);
        temp = strtok(NULL, s);
    }
    return file;
}


int main(int argc, char *argv[]){

    int port, sock;
    char buf[BUFF];
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

    strcpy(buf,directory);

    // write the path we want to copy on client's file system
    if (write(sock, buf, BUFF) < 0)
        perror_exit("write");

    int dwse = 1;
    while(dwse){
                
        // read the name of the file
        if(read(sock,buffer, BUFF) < 0){
            perror("read");
        }
        printf("%s\n", buffer);
        char* file = separate(buffer);
        printf("%s\n",file);

        /*********************************************************/
        // create a file in the current dir.
        // so we copy the (server) file to the /file
        FILE *write_file = fopen(file, "w");
        if ( write_file == NULL){   
            printf("Error! Could not open file\n"); 
            exit(EXIT_FAILURE); 
        } 
        
        while(recv(sock,buffer_read,BUFF,0) > 0){
            printf("AAAAAAAAAA now i'm geting the file's data\n");
            printf(">>THE DATA:\n\t%s //telos\n", buffer_read);
            fprintf(write_file,"%s", buffer_read);
        }
        
        fclose(write_file);

    }

    return 0;
}


void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}