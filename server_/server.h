#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <sys/wait.h>	     /* sockets */
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <netdb.h>	         /* gethostbyaddr */
#include <unistd.h>	         /* fork */		
#include <stdlib.h>	         /* exit */
#include <ctype.h>	         /* toupper */
#include <signal.h>          /* signal */
#include <pthread.h>         /* For threads */
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define perror2(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

/***************************************************************************************/

pthread_mutex_t mtx;
pthread_mutex_t mtx_2;
pthread_mutex_t mtx_3;
pthread_mutex_t mtx_4;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
pthread_cond_t cvar;

/***************************************************************************************/

struct thread_args {
    int f_socket;
    int s_size;
    int bl_size;
};

int sock_copy = 0;

/***************************************************************************************/

#define BUFF 4092

typedef struct {
        char* data[BUFF];
        int start;
        int end;
        int count;
} queue_t;

queue_t oyra;

void initialize(queue_t *oyra) {
    oyra->start = 0;
    oyra->end = -1;
    oyra->count = 0;
}

void place(queue_t *oyra, char* data, int size) {
    pthread_mutex_lock(&mtx);
    while (oyra->count >= size){
        //printf(">>Buffer Full!\tI'm waiting a work thread\n");
        pthread_cond_signal(&cvar);
        pthread_cond_wait(&cond_nonfull, &mtx);
    }
    oyra->end = (oyra->end + 1) % size;
    oyra->data[oyra->end] = data;
    oyra->count++;
    pthread_mutex_unlock(&mtx);
}

char* obtain(queue_t * oyra, int size) {
    char* data;
    pthread_mutex_lock(&mtx);
    while (oyra->count <= 0) {
        //printf(">>Buffer Empty!\tI'm waiting the com-thread\n");
        pthread_cond_wait(&cond_nonempty, &mtx);
    }
    data = oyra->data[oyra->start];
    oyra->start = (oyra->start + 1) % size;
    oyra->count--;
    pthread_mutex_unlock(&mtx);
    return data;
}

/***************************************************************************************/

int num_of_files = 0;

int find_num_of_files(char* file){
    DIR *folder;
    struct dirent *entry;
    char new_file[BUFF];

    folder = opendir(file);
    if(folder == NULL){
        perror("Unable to read directory");
        return(1);
    }
    while( (entry=readdir(folder)) ){
        if (entry->d_type == DT_DIR){
            if( !(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0) ){
                sprintf(new_file, "%s/%s", file, entry->d_name );
                find_num_of_files(new_file);
            }
        }
        else if (entry->d_type == DT_REG){
            num_of_files++;
        }
    }
    closedir(folder);
    return 0;
}

/***************************************************************************************/

int place_the_files(char* file, pthread_t thread, int max_size){
    DIR *folder;
    struct dirent *entry;
    char new_file[BUFF];
    folder = opendir(file);
    if(folder == NULL){
        perror("Unable to read directory");
        return(1);
    }
    while( (entry=readdir(folder)) ){
        if (entry->d_type == DT_DIR){
            if( !(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0) ){
                sprintf(new_file, "%s/%s", file, entry->d_name );
                place_the_files(new_file,thread,max_size);
            }
        }
        else if (entry->d_type == DT_REG){
            pthread_mutex_lock(&mtx_3);
        
            //sprintf(new_file, "%s/%s", file, entry->d_name);
            
            place(&oyra, entry->d_name, max_size);
            printf("[Thread %ld]: Adding file <%s> to the queue…\n", thread, entry->d_name);
            num_of_files--;
            pthread_cond_signal(&cond_nonempty);
            pthread_mutex_unlock(&mtx_3);
        }
    }

    closedir(folder);
    return 0;
}

/*******************************************************************************************/

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

/*******************************************************************************************/








/***************************************************************************************/

void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}

// Wait for all dead child processes
void sigchld_handler (int sig) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

#endif