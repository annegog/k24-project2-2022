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

#define BUFF 4096

#define perror2(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

/***************************************************************************************/

pthread_mutex_t mtx;
pthread_mutex_t mtx_2;
pthread_mutex_t mtx_3;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
pthread_cond_t cvar;

/***************************************************************************************/

void perror_exit(char *message);
void sigchld_handler (int sig);

/***************************************************************************************/
struct thread_args {
    int f_socket;
    int s_size;
};
/***************************************************************************************/


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
        printf(">>Buffer Full!\tI'm waiting a work thread to take a file from the queue\n");
        pthread_cond_signal(&cvar);
        pthread_cond_wait(&cond_nonfull, &mtx);
    }
    oyra->end = (oyra->end + 1) % size;
    oyra->data[oyra->end] = data;
    oyra->count++;
    printf("here- I just place a file <%s> in the oyra.\n", data);
    pthread_mutex_unlock(&mtx);
}

char* obtain(queue_t * oyra, int size) {
    char* data;
    pthread_mutex_lock(&mtx_3);
    printf("aaaaa\n");
    while (oyra->count <= 0) {
        printf(">>Buffer Empty!\tI'm waiting the com-thread to push a file in the queue\n");
        pthread_cond_wait(&cond_nonempty, &mtx_3);
    }
    data = oyra->data[oyra->start];
    oyra->start = (oyra->start + 1) % size;
    printf("Here- I just took a file from the oyra\n");
    oyra->count--;
    pthread_mutex_unlock(&mtx_3);
    return data;
}

/***************************************************************************************/
int pos = 0;


int find_the_files(char* file, pthread_t thread, int max_size){
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
                find_the_files(new_file,thread,max_size);
            }
        }
        else if (entry->d_type == DT_REG){
            pos++;
            sprintf(new_file, "%s/%s", file, entry->d_name);
            place(&oyra,new_file,max_size);
            printf("[Thread %ld]: Adding file <%s> to the queue…\n", thread, new_file);
            //pthread_cond_signal(&cond_nonempty);
            printf("pos= %d\n", pos);
        }
        else{
            printf(">> i make pos=0\n");
            pos = 0;
        }
    }
    closedir(folder);
    return 0;
}

/****************************** communication thread ***************************************/

void *communication_thread(void *argp){ 
    //printf("I am the newly created communication thread %ld\n", pthread_self());
    char buff[BUFF];    
    struct thread_args *args = (struct thread_args *) argp;

    // να διβάσει το path που εστειλε ο client
    if(read(args->f_socket, buff, BUFF) < 0){
        perror_exit("read");
    }
    // να βάλει το αρχειο στην ουρά
    // αν η ουρά ειναι γεμάτη περιμένει
    
    printf("[Thread %ld]: About to scan the directory %s\n", pthread_self(), buff);
    find_the_files(buff,pthread_self(),args->s_size);
    pthread_cond_signal(&cond_nonempty);
    //pthread_cond_signal(&cvar);

    printf(">>HEEEEELLOOOO\n");

    free(argp);
    pthread_exit(NULL);
}

/***************************************** worker thread ***********************************/

void *worker_thread(void *arg){
    int err;
    printf("Just created a worker thread %ld\n", pthread_self());
    
    struct thread_args *args = (struct thread_args *) arg;
    int size = args->s_size;

    pthread_cond_wait(&cvar, &mtx_2); //* Wait for signal

    // if (err = pthread_mutex_lock(&mtx_2)) {            /* Lock mutex */
    //     perror2("pthread_mutex_lock", err); exit(1); }
    // printf("AAA Thread %ld: Locked the mutex\n", pthread_self());

    printf(">>hey 2!!! counter = %d > %d\n", oyra.count, size);
    // for(int i=0; i < size; i++){
    //     printf(" >>>>> %s\n", oyra.data[i]);
    // }
    while (oyra.count > 0) {
        printf("consumer: %s\n", obtain(&oyra,size));
        pthread_cond_signal(&cond_nonfull);
    }

    // if (err = pthread_mutex_unlock(&mtx)) {                  /* Unlock mutex */
    //     perror2("pthread_mutex_unlock", err); exit(1); }
    // printf("Thread %ld: Unlocked the mutex\n", pthread_self());

    printf("??? ARE YOU HERE ???\n");
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

    pthread_cond_init(&cvar, 0); // initialize condition variable

    initialize(&oyra);
    pthread_mutex_init(&mtx, 0);
    pthread_mutex_init(&mtx_2, 0);
    pthread_mutex_init(&mtx_3, 0);
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
    args->f_socket = newsock;
    args->s_size = queue_size;

    /************************** create the worker threads **********************************/
    
    // make a pool- array for the worker threads
    pthread_t * pool_threads_array;
    pool_threads_array = malloc (sizeof(pthread_t) * thread_pool_size);
    
    for(pthread_t i=0; i<thread_pool_size; i++){
        if (err = pthread_create(&thr_work, NULL, worker_thread, args)){
            perror2("pthread_create", err);
            exit(1);
        }
        printf("Thread %ld: Created thread %ld\n", pthread_self(), thr_work);

        pool_threads_array[i]= thr_work;

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
        args->s_size = queue_size;

        if (err = pthread_create(&thr_com, NULL, communication_thread, args)){
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
    pthread_cond_destroy(&cond_nonempty);
    pthread_cond_destroy(&cond_nonfull);
    pthread_mutex_destroy(&mtx);
    pthread_mutex_destroy(&mtx_2);
    pthread_mutex_destroy(&mtx_3);
    
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