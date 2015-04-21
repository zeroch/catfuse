/* 
 *  A threaded server
 *  by Martin Broadhurst (www.martinbroadhurst.com)
 *  Compile with -pthread
 */

#include "thread_server.h"


void *handle(void *pnewsock)
{
    /* send(), recv(), close() */
    int n;
    char sendBuf[2000], client_message[2000];
    int sock = *(int*) pnewsock;
    while( (n = recv(sock, client_message, 2000, 0)) > 0)
    {
        puts("message received");
        puts(client_message);
        strcpy(sendBuf, client_message);
        printf("sending message: echo %s", sendBuf);
        send(sock, client_message, n, 0);
        if (n == 0)
        {
            puts("client disconnected");
            close(sock);
        }else
        {
            perror("recv failed");
        }
    }


    return NULL;
}

int socket_init(void )
{
    int sock;
    pthread_t thread;

    struct addrinfo hints, *res;
    int reuseaddr = 1; /* True */

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    /* Create the socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        perror("socket");
        return 1;
    }
    puts("Socket created");

    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }

    /* Bind to the address */
    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        return 0;
    }
    puts("bind done");

    freeaddrinfo(res);

    /* Listen */
    if (listen(sock, BACKLOG) == -1) {
        perror("listen");
        return 0;
    }
    puts("Waiting for incoming connections...");

    /* Main loop */
    while (1) {
        size_t size = sizeof(struct sockaddr_in);
        struct sockaddr_in their_addr;
        size_t newsock = accept(sock, (struct sockaddr*)&their_addr, &size);
        if (newsock == -1) {
            perror("accept");
        }
        else {
            printf("Got a connection from %s on port %d\n", 
                    inet_ntoa(their_addr.sin_addr), htons(their_addr.sin_port));
            if (pthread_create(&thread, NULL, handle, &newsock) != 0) {
                fprintf(stderr, "Failed to create thread\n");
            }
            return 1;
        }
    }

}
