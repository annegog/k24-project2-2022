// Client

#include <stdio.h>
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <unistd.h>          /* read, write, close */
#include <netdb.h>	         /* gethostbyaddr */
#include <stdlib.h>	         /* exit */
#include <string.h>	         /* strlen */

void perror_exit(char *message);


main(int argc, char *argv[]){






}


void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}