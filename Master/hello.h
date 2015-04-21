#ifndef _HELLO_H_
#define _HELLO_H_
#include "thread_server.h"
#include "filetransfer.h"
#include "list.h"
#include "testdb.h"
#include "md5.h"

#define MAX_RETRY 3
#define MAX_NAMELEN 255

extern void MD5_Init(MD5_CTX *ctx);
extern void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
extern void MD5_Final(unsigned char *result, MD5_CTX *ctx);
int UpdateList(char* path);


#endif


