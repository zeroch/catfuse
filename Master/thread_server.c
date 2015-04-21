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



int kick_start(void )
{
    int clientSocket;
    char buffer[2000];
    char read_buf[2000];
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    /*---- Create the socket. The three arguments are: ----*/
    /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

    /*---- Configure settings of the server address struct ----*/
    /* Address family = Internet */
    serverAddr.sin_family = AF_INET;
    /* Set port number, using htons function to use proper byte order */
    serverAddr.sin_port = htons(DB_PORT);
    /* Set IP address to localhost */
    serverAddr.sin_addr.s_addr = inet_addr(DB_IP);
    /* Set all bits of the padding field to 0 */
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    /*---- Connect the socket to the server using the address struct ----*/
    addr_size = sizeof serverAddr;
    connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

    /*---- Read the message from the server into the buffer ----*/

    strcpy(buffer, "6,   / I am walking up!");
    send(clientSocket, buffer, 2000, 0);
    recv(clientSocket, read_buf, 2000, 0);

    /*---- Print the received message ----*/
    printf("Data received: %s",buffer);   

    printf("------------ Kick starting DataBase -------\n");

}