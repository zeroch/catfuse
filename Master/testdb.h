#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>  

void getContent(char* obj_id, char* reply);
void postContent(char* obj_id,int version,char* hash,char* reply);
void delContent(char* obj_id,char* reply);
void listContent(char* reply);
