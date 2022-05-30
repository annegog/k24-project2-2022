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

#define BUFF 4096

void perror_exit(char *message);

int find_the_file(char* dir, char* file){
    int fd;
    DIR *folder;
    struct dirent *entry;
    char new_folder[BUFF];
    folder = opendir(dir);
    // if the is no
    if(folder == NULL){
        perror("Unable to read directory");
    }
    else{
        fd = mkdir(dir,S_IRWXU);
        folder = opendir(dir);
        if(folder == NULL){
            perror("Unable to read directory");
        }
    }
    if( (entry=readdir(folder))){
        if (entry->d_type == DT_DIR){
            if( !(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0) ){
                sprintf(new_folder, "%s/%s", dir, entry->d_name );
                find_the_file(new_folder, file);
            }
        }
        if (entry->d_type == DT_REG){
            if( entry->d_name == file){
                if (remove(file) == 0) {
                    printf("The file is deleted successfully.");
                } 
                else {
                    printf("The file is not deleted.");
                }
            }
        }
    }
    return fd;
}


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

int write_data ( int fd, char* message ){/* Write formated data */
	char temp; int length = 0;
	length = strlen(message) + 1;	/* Find length of string */
	temp = length;
	if( write (fd, &temp, 1) < 0 )	/* Send length first */
		exit (-2);
	if( write (fd, message, length) < 0 )	/* Send string */
		exit (-2);
	return length;		/* Return size of string */
}

int read_data (int fd, char *buffer){/* Read formated data */
	char temp;int i = 0, length = 0;
	if ( read ( fd, &temp, 1 ) < 0 )	/* Get length of string */
		exit (-3);
	length = temp;
	while ( i < length )	/* Read $length chars */
		if ( i < ( i+= read (fd, &buffer[i], length - i)))
			exit (-3);
	return i;	/* Return size of string */
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

    
    while(1){
        // write the path we want to copy on client's file system
        if (write(sock, buf, BUFF) < 0)
            perror_exit("write");

        // read the name of the file
        // if(read(sock,buffer, BUFF) < 0){
        //     perror("read");
        // }
        read_data(sock,buffer);
        
        char* file = separate(buffer);
        printf("File: %s\n",file);
        usleep(1000);
        
        /*********************************************************/
        
        // create a file in the current dir.
        // so we copy the (server) file to the /file
        
        int fd = find_the_file(directory,file);
        FILE *write_file = fopen(file, "w");
        if ( write_file == NULL){   
            printf("Error! Could not open file\n"); 
            exit(EXIT_FAILURE); 
        }

        while( recv(sock, buffer_read, BUFF, 0) > 0){
            //printf(">>THE DATA:\n%s //telos\n", buffer_read);
            fprintf(write_file,"%s", buffer_read);
            sleep(1);
        }usleep(10000);
        printf("out of the while recv\n");

        fprintf(write_file,"skata...out of the while!");
        
        fclose(write_file);
        close(fd);
    }

    return 0;
}


void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}