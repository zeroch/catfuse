#include "testdb.h"

//#define DB_IP "128.61.24.181"
#define DB_IP "127.0.0.1"

void delContent(char* obj, char* server_reply){
  struct sockaddr_in server;
  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  char message[255];
  sprintf(message,"2,%s",obj);
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

void listContent(char* server_reply,int replica_id){
  struct sockaddr_in server;
  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  char message[255];
  sprintf(message,"3,%d",replica_id);
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

void postContent(char* obj, int version, char* hash,char* server_reply){
  struct sockaddr_in server;
  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  char message[1024];
  sprintf(message,"0,%s,%d,%s",obj,version,hash);

  if (sockfd < 0){
    puts("Crete Socket Error");
    return;
  }
  server.sin_addr.s_addr = inet_addr(DB_IP);
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
  server.sin_addr.s_addr = inet_addr(DB_IP);
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

void sendList(char* objList, char* server_reply, int replica_id){
  struct sockaddr_in server;
  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  char message[2000];
  sprintf(message,"4,replica_id,%d",objList);
  
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

int regReplica(){
  char server_reply[2000];
  struct sockaddr_in server;
  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  char message[2000];
  sprintf(message,"5,register");

  if (sockfd < 0){
    puts("Crete Socket Error");
    return -1;
  }
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons(12345);

  if (connect(sockfd,(struct sockaddr*)&server,sizeof(server))<0){
    puts("Connection Error");
    return -1;
  }
  if(send(sockfd,message,strlen(message),0)<0){
    puts("Send failed");
    return -1;
  }
  if(recv(sockfd,server_reply,2000,0)<0){
    puts("Receive Failed");
    return -1;
  }
  close(sockfd);

  
  if(strcmp(server_reply,"REGISTER_ERROR")==0||strcmp(server_reply,"REPLICA_TABLE_FULL")==0){
    return -1;
  }

  int replica_id = atoi(server_reply);
  
  return replica_id;

}
