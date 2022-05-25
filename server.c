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

#include "ADTDeque.h"
#include "ADTDeque.c"
#include "pool.h"

#define BUFF 4096

#define perror2(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

void perror_exit(char *message);
void sigchld_handler (int sig);

void *communication_thread(void *argp){ /* Thread function */
    printf("I am the newly created communication thread %ld\n", pthread_self());
    int sock = *(int*)argp;
    char buff[BUFF];
    if(read(sock, buff, BUFF) < 0){
        perror_exit("read");
    }   
    printf("I read: %s\n", buff);


    // να διβάσει το path που εστειλε ο client
   
    //deque_insert_first(deque, a);
    // να βάλει το αρχελιο στην ουρά
    // αν η ουρά ειναι γεμάτη περιμένει

}

int main(int argc, char *argv[]){
    
    int port, sock, newsock;
    int thread_pool_size=0, queue_size=0, block_size=0;
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct hostent *rem;
    
    pthread_t thr;
    int err, status;
  
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

    Deque deque = deque_create(queue_size,NULL);

    

    // Create socket
    printf("create socket\n");
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    server.sin_family = AF_INET;       /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);      /* The given port */
    /* Bind socket to address */
    printf("Bind socket to address\n");
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("bind");
    /* Listen for connections */
    if (listen(sock, 200) < 0) perror_exit("listen");
    printf("Listening for connections to port %d\n", port);
    while(1){ 
        clientlen = sizeof(client);
    	if ((newsock = accept(sock, clientptr, &clientlen)) < 0) 
            perror_exit("accept");
    	printf("Accepted connection from localhost\n");
        
        int* pr_new = malloc(sizeof(int));
        *pr_new= newsock;
        if (err = pthread_create(&thr, NULL, communication_thread, pr_new)) { /* New thread */
            perror2("pthread_create", err);
            exit(1);
        }
    	
    	//close(newsock); //------- parent closes socket to client
    }

    // delete the com=thread
    if (err = pthread_join(thr, (void **) &status)) { /* Wait for thread */
        perror2("pthread_join", err); /* termination */
        exit(1);
    }
    printf("Thread %ld exited with code %d\n", thr, status);
    pthread_exit(NULL);


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