/* 
 *  A threaded server
 *  by Martin Broadhurst (www.martinbroadhurst.com)
 *  Compile with -pthread
 */

#include "thread_server.h"

/*  assume input a buff with size 2000
    use ',' as spliter. 

*/
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;

}

void pull_parser(char *msg)
{
    char** tokens;

    printf("msg=[%s]\n\n", msg);

    tokens = str_split(msg, ',');
    char  url[100];
    strcpy(url, REMOTE_URL);

    char qolon = ':';//character to search
    

    if (tokens)
    {
        int i;
        for (i = 0; *(tokens + i); i++)
        {


            printf("string = %s\n", *(tokens + i));
            // strcat(url, *(tokens + i));
            
            char *quotPtr = strchr( *(tokens + i), qolon);

            if (quotPtr == NULL)
            {
                printf("debug: format Error\n");

                free(*(tokens + i));
                continue;
            }
            int position = quotPtr - *(tokens + i);
            // strncpy(quotPtr, '\0', 1);
            char f_name[100], m_location[20];
            memset(f_name, 0, 100);
            memset(m_location, 0, 20);

            strncpy(f_name, *(tokens + i), position);
            strcpy(m_location, *(tokens + i)+position+1);

            printf("file=[%s]\n", f_name);
            // use FTP at here. last file is dead. FIXME
            char t_url[100];
            memset(t_url, 0 , 100);
            strcpy(t_url,url);
            strcat(t_url,m_location);
            printf("debug: location at %s\n", m_location);
            printf("debug: location in url at %s\n", t_url);
            transfer_get(t_url, f_name);
            free(*(tokens + i));
        }
        printf("\n");
        free(tokens);
    }
}

void push_parser(char *msg)
{
   
    char** tokens;

    printf("msg=[%s]\n\n", msg);

    tokens = str_split(msg, ',');
    char * url[100];
    strcpy(url, REMOTE_URL);
    if (tokens)
    {
        int i;
        for (i = 0; *(tokens + i); i++)
        {
            if (i == 0)
            {
                printf("replica at %s\n", *(tokens + i));
                strcat(url, *(tokens + i));
                continue;
            }
            printf("file=[%s]\n", *(tokens + i));
            // use FTP at here. last file is dead. FIXME
            printf("debug: %s\n", url);
            transfer_put(url, *(tokens + i));
            free(*(tokens + i));
        }
        printf("\n");
        free(tokens);
    }

}


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

        // parse the message into src, filename, ...
        char cmd[5];
        strncpy(cmd, client_message, 5);
        strcpy(client_message, client_message+5);
        if (!strcmp(cmd, "push,"))
        {
            push_parser(client_message);
        }else if (!strcmp(cmd, "pull,"))
        {
            pull_parser(client_message);

        } else {
            
            strcpy(client_message, "Test Connect");
            send(sock, client_message, n, 0);


        }



        // strcpy(sendBuf, client_message);
        // printf("sending message: echo %s", sendBuf);
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

    printf("I am reading to kick start DataBase\n");

    /*---- Create the socket. The three arguments are: ----*/
    /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    printf("DEBUG: I am dead here \n");

    /*---- Configure settings of the server address struct ----*/
    /* Address family = Internet */
    serverAddr.sin_family = AF_INET;
    /* Set port number, using htons function to use proper byte order */
    serverAddr.sin_port = htons(DB_PORT);
    printf("DEBUG: I am dead here \n");
    /* Set IP address to localhost */
    serverAddr.sin_addr.s_addr = inet_addr(DB_IP);
    printf("DEBUG: I am dead here \n");
    /* Set all bits of the padding field to 0 */
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  
    printf("DEBUG: I am dead here \n");

    /*---- Connect the socket to the server using the address struct ----*/
    addr_size = sizeof serverAddr;
    if (connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size)){
        puts("connections Error\n");
        return;
    }



    printf("DEBUG: I am dead here \n");

    /*---- Read the message from the server into the buffer ----*/

    strcpy(buffer, "6,   / I am walking up!");
    printf("DEBUG: I am dead here 0\n");
    if(send(clientSocket, buffer, 2000, 0)){
        puts("Send failed");
        return;
    }
    printf("DEBUG: I am dead here 1 \n");
    if (recv(clientSocket, read_buf, 2000, 0))
    {
        puts("Receive Failed");
        return;
    }
    
    printf("DEBUG: I am dead here 2\n");

    /*---- Print the received message ----*/
    printf("Data received: %s",buffer);   

    printf("------------ Kick starting DataBase -------\n");

    return 0;
}