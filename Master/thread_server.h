#include <stdio.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#define PORT    "3000" /* Port to listen on */
#define BACKLOG     10  /* Passed to listen() */

void *handle(void *pnewsock);
int socket_init(void );

