#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>  

void getContent(char* obj_id, char* reply);
void postContent(char* obj_id,int version,unsigned char* hash,char* reply);
