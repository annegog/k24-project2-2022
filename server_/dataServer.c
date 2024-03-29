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
#include <fcntl.h>

#include "server.h"

/****************************** communication thread ***************************************/

void *communication_thread(void *argp){ 
    //printf("I am the newly created communication thread %ld\n", pthread_self());
    char buff[BUFF];    
    struct thread_args *args = (struct thread_args *) argp;

    // read the path from the client
    if(read(args->f_socket, buff, BUFF) < 0){
        perror_exit("read");
    }
    find_num_of_files(buff);
    printf("\n[Thread %ld]: About to scan the directory %s\n", pthread_self(), buff);
    
    while(num_of_files > 0){
        place_the_files(buff,pthread_self(),args->s_size);
    }
    if(num_of_files == 0){
        pthread_cond_signal(&cvar);
    }
    free(argp);
    return 0;
}

/***************************************** worker thread ***********************************/

void *worker_thread(void *arg){
    // printf("Just created a worker thread %ld\n", pthread_self());
    pthread_mutex_lock(&mtx_2);
    struct thread_args *args = (struct thread_args *) arg;
    int size = args->s_size;
    int socket;

    pthread_cond_wait(&cvar, &mtx_2); // wait for signal
    
    char* file;
    socket = sock_copy;

    while (oyra.count > 0 || num_of_files > 0){
        file = obtain(&oyra,size);
        pthread_cond_signal(&cond_nonfull);
        printf("{Thread %ld}: Received task: <%s, %d>\n", pthread_self(), file, socket);
        pthread_mutex_lock(&mtx_4);
        send_d(socket, file, args->bl_size);
        pthread_mutex_unlock(&mtx_2);
    }

    printf(">> No more files to read. I don't know what to dooo now!:( \n");
    close(args->f_socket);
    pthread_exit(0);
}

/***************************************************************************************/


int main(int argc, char *argv[]){
    
    int port, sock, newsock;
    int thread_pool_size=0, queue_size=0, block_size=0;
    
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    
    pthread_t thr_com, thr_work;
    int err, status;

    pthread_cond_init(&cvar, 0); // initialize condition variable

    initialize(&oyra);
    pthread_mutex_init(&mtx, 0);
    pthread_mutex_init(&mtx_2, 0);
    pthread_mutex_init(&mtx_3, 0);
    pthread_mutex_init(&mtx_4, 0);
    pthread_cond_init(&cond_nonempty, 0);
    pthread_cond_init(&cond_nonfull, 0);

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
            printf("block size: %d\n\n", block_size);
        }
    }

    // Reap dead children asynchronously
    signal(SIGCHLD, sigchld_handler);
    
    struct thread_args *args = malloc (sizeof (struct thread_args));
    //args->f_socket = newsock;
    args->s_size = queue_size;
    args->bl_size = block_size;

    /************************** create the worker threads **********************************/
    
    // make a pool- array for the worker threads
    pthread_t * worker_thr_id = malloc(thread_pool_size * sizeof(*worker_thr_id));
    
    for(pthread_t i=0; i<thread_pool_size; i++){
        if ((err = pthread_create(&worker_thr_id[i], NULL, worker_thread, args))){
            perror2("pthread_create", err);
            exit(1);
        }
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
        
        args->f_socket = newsock;
        sock_copy = newsock;

        if ((err = pthread_create(&thr_com, NULL, communication_thread, args))){
            perror2("pthread_create", err);
            exit(1);
        }
        //printf("Thread %ld: Created thread %ld\n", pthread_self(), thr_com);

    }
    
    /***************************************************************************************/
    
    // delete the com-thread
    if ((err = pthread_join(thr_com, (void **) &status)) ) { /* Wait for thread */
        perror2("pthread_join", err); /* termination */
        exit(1);
    }
    printf("Thread %ld exited with code %d\n", thr_com, status);
    pthread_exit(NULL);

    for(pthread_t i=0; i<thread_pool_size; i++){
        free(worker_thr_id);
    }
    pthread_cond_destroy(&cond_nonempty);
    pthread_cond_destroy(&cond_nonfull);
    pthread_mutex_destroy(&mtx);
    pthread_mutex_destroy(&mtx_2);
    pthread_mutex_destroy(&mtx_3);
    pthread_mutex_destroy(&mtx_4);
    
    return 0;
}
