#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>  

void getContent(char*,char*);
void postContent(char*,int,unsigned char*,char*);

void postContent(char* obj, int version, unsigned char* hash,char* server_reply){
    struct sockaddr_in server;
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    char message[1024];
    sprintf(message,"0,%s,%d,%s",obj,version,hash);

    if (sockfd < 0){
        puts("Crete Socket Error");
        return;
    }
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);

    if (connect(sockfd,(struct sockaddr*)&server,sizeof(server))<0){
        puts("Connection Error");
        return;
    }
    if(send(sockfd,message,strlen(message),0)<0){
        puts("Send failed");
        return;
    }
    if(recv(sockfd,server_reply,2000,0)<0){
        puts("Receive Failed");
        return;
    }
    close(sockfd);
}

void getContent(char* obj, char* server_reply){
    struct sockaddr_in server;
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    char message[255];
    sprintf(message,"1,%s",obj);
    //char* server_reply;

    if (sockfd < 0){
        puts("Crete Socket Error");
        return;
    }
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);

    if (connect(sockfd,(struct sockaddr*)&server,sizeof(server))<0){
        puts("Connection Error");
        return;
    }
    if(send(sockfd,message,strlen(message),0)<0){
        puts("Send failed");
        return;
    }
    if(recv(sockfd,server_reply,2000,0)<0){
        puts("Receive Failed");
        return;
    }
    close(sockfd);
    //return server_reply;

} 
int main(int argc,char* argv[]){
    

    char objID[255]={};
    unsigned char hashPath1[2000]={};
    char output[2000]={};
    unsigned char hashPath2[2000]={};

    strcpy(objID,"host");
    strcpy(hashPath1,"hash");
    int version = 1;



    postContent(objID,version,hashPath1,output);
    printf("%s\n",output);



    getContent(objID, hashPath2);
    printf("%s\n",hashPath2);



/*    message = "1,1";
    if(send(sockfd,message,strlen(message),0)<0){
        puts("Send failed");
        return 1;
    }
    if(recv(sockfd,server_reply1,2000,0)<0){
        puts("Receive Failed");
        return 1;
    }
    puts(server_reply1);
*/

    return 0;
}
