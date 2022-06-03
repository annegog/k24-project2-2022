#ifndef CLIENT_H_
#define CLIENT_H_

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

/***************************************************************************************/

void find_the_file(char* dir, char* file){
    DIR *folder;
    struct dirent *entry;
    char new_folder[BUFF];
    folder = opendir(dir);
    // if the is no
    if(folder == NULL){
        return;
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
                else{
                    printf("The file is not deleted.");
                }
            }
        }
    }
}

/***********************************************************************/

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

/*************************************************************/

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

/*************************************************************/

void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}

#endif