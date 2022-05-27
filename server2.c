// Server
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


#include "ADTDeque.h"
#include "ADTDeque.c"

#define BUFF 4096

#define perror2(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

/***************************************************************************************/

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar;                                             /* Condition variable */

/***************************************************************************************/

void perror_exit(char *message);
void sigchld_handler (int sig);

/***************************************************************************************/

int pos = 0;

int find_the_files(char* file, pthread_t thread){
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
                find_the_files(new_file,thread);
            }
        }
        else{
            pos++;
            sprintf(new_file, "%s/%s", file, entry->d_name);
            
            printf("[Thread %ld]: Adding file <%s> to the queue…\n", thread, new_file);
            //deque_insert_first(queue, new_file);
            printf("pos=%d\n", pos);
            
            
            //assert(deque_get_at(queue, pos) == new_file);
        }
    }
    closedir(folder);
    return 0;
}

/****************************** communication thread ***************************************/

void *communication_thread(void *argp){ 
    //printf("I am the newly created communication thread %ld\n", pthread_self());
    int sock = *(int*)argp;
    char buff[BUFF];

    // να διβάσει το path που εστειλε ο client
    if(read(sock, buff, BUFF) < 0){
        perror_exit("read");
    }
    
    // να βάλει το αρχειο στην ουρά
    // αν η ουρά ειναι γεμάτη περιμένει
    printf("[Thread %ld]: About to scan the directory %s\n", pthread_self(), buff);
    find_the_files(buff,pthread_self());

    free(argp);
    pthread_exit(NULL);
}

/***************************************** worker thread ***********************************/

void *worker_thread(void *arg){
    int err;

    //printf("Just created a worker thread %ld\n", pthread_self());
    //printf("Thread %ld: Waiting for signal\n", pthread_self());
    pthread_cond_wait(&cvar, &mtx); //* Wait for signal

    if (err = pthread_mutex_lock(&mtx)) {            /* Lock mutex */
        perror2("pthread_mutex_lock", err); exit(1); }
    printf("AAA Thread %ld: Locked the mutex\n", pthread_self());

    // META
    // gia na to bgalw apo to wait
    /* Wake up one thread waiting for condition variable COND.  */
    pthread_cond_signal(&cvar);                          /* Awake thread */
    printf("Thread %ld: Sent signal\n", pthread_self());

    if (err = pthread_mutex_unlock(&mtx)) {                  /* Unlock mutex */
        perror2("pthread_mutex_unlock", err); exit(1); }
    printf("LALALLA Thread %ld: Unlocked the mutex\n", pthread_self());

    pthread_exit(NULL);
}

/***************************************************************************************/


int main(int argc, char *argv[]){
    
    int port, sock, newsock;
    int thread_pool_size=0, queue_size=0, block_size=0;
    
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct hostent *rem;
    
    pthread_t thr_com, thr_work;
    int err, status;

    pthread_cond_init(&cvar, NULL); // initialize condition variable

    /******************************** arguments ********************************************/
    
    if (argc != 9){
        // ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>
        printf("Worng arguments\n");
        exit(1);
    }
    printf("Server’s parameters are:\n");
    for (int i = 0; i < argc; i++){
        if (strcmp(argv[i], "-p") == 0){
            port = atoi(argv[i + 1]);
            printf("port: %d\n", port);
        }
        if (strcmp(argv[i], "-s") == 0){
            thread_pool_size = atoi(argv[i + 1]);
            printf("thread pool size: %d\n", thread_pool_size);
        }
        if (strcmp(argv[i], "-q") == 0){
            queue_size = atoi(argv[i + 1]);
            printf("queue size: %d\n",queue_size);
        }
        if (strcmp(argv[i], "-b") == 0){
            block_size = atoi(argv[i + 1]);
            printf("block size: %d\n", block_size);
        }
    }

    // Reap dead children asynchronously
    signal(SIGCHLD, sigchld_handler);

    /************************** create the worker threads **********************************/
    
    // make a pool- array for the worker threads
    pthread_t * pool_threads_array;
    pool_threads_array = malloc (sizeof(pthread_t) * thread_pool_size);
    
    for(pthread_t i=0; i<thread_pool_size; i++){
        // Lock mutex
        if (err = pthread_mutex_lock(&mtx)){                           
            perror2("pthread_mutex_lock", err); exit(1); }
        //printf("Thread %ld: Locked the mutex\n", pthread_self());

        if (err = pthread_create(&thr_work, NULL, worker_thread, NULL)){
            perror2("pthread_create", err);
            exit(1);
        }
        printf("Thread %ld: Created thread %ld\n", pthread_self(), thr_work);

        pool_threads_array[i]= thr_work;

        // Unlock mutex
        if (pthread_mutex_unlock(&mtx)) {                     
            perror("pthread_mutex_unlock"); exit(1); }
        //printf("Thread %ld: Unlocked the mutex\n", pthread_self());
    }

    /***************************************************************************************/
    // Create socket
    // printf("create socket\n");
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    server.sin_family = AF_INET;       /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);      /* The given port */

    /* Bind socket to address */
    //printf("Bind socket to address\n");
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("bind");
    /* Listen for connections */
    if (listen(sock, 200) < 0) 
        perror_exit("listen");
    printf("Listening for connections to port %d\n", port);
    while(1){
        clientlen = sizeof(client);
    	if ((newsock = accept(sock, clientptr, &clientlen)) < 0) 
            perror_exit("accept");
    	printf("Accepted connection from localhost\n");
        
        /************************* making the communication thread *************************/
        
        int* pr_new = malloc(sizeof(int));
        *pr_new= newsock;

        if (err = pthread_create(&thr_com, NULL, communication_thread, pr_new)){
            perror2("pthread_create", err);
            exit(1);
        }
        printf("Thread %ld: Created thread %ld\n", pthread_self(), thr_com);

    	//close(newsock); //------- parent closes socket to client
    }
    
    /***************************************************************************************/
    
    // delete the com-thread
    if (err = pthread_join(thr_com, (void **) &status)) { /* Wait for thread */
        perror2("pthread_join", err); /* termination */
        exit(1);
    }
    printf("Thread %ld exited with code %d\n", thr_com, status);
    pthread_exit(NULL);

    free(pool_threads_array);
    return 0;
}


void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}

// Wait for all dead child processes
void sigchld_handler (int sig) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
}