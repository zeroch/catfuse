/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "thread_server.h"
#include "filetransfer.h"
#include <dirent.h>

#include "list.h"
#include "testdb.h"
#include "md5.h"

#define MAX_RETRY 3
#define MAX_NAMELEN 255

extern void MD5_Init(MD5_CTX *ctx);
extern void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
extern void MD5_Final(unsigned char *result, MD5_CTX *ctx);

FILE* log_file;

int replica_id = -1;
static struct list_node entries;

struct ou_entry {
  struct list_node node;
  mode_t mode;
  struct timespec tv;
  char md5_hash[33];
  char name[MAX_NAMELEN + 1];
};

struct cache_index{
  char name[MAX_NAMELEN + 1];
  char version[20];
  char md5_hash[32];
};

static void fullPath(char fpath[MAX_NAMELEN], const char * path)
{
  strcpy(fpath, ROOT_DIR);
  strncat(fpath, path, MAX_NAMELEN);
}


void writeLogFile(char* data){
  #ifdef DEBUG

  fprintf(log_file, "%s\n", data);

  #endif
}

int getDBList(){
  
  if(replica_id==-1){
    replica_id = regReplica();
  }


  struct list_node *n;
  struct list_node *p;
  int res;
  
  //get list                                                                  
  char list_reply[2000];
  struct cache_index* my_cache_list[30] = { 0 };
  /* currently use a static array of size 30, if have time may have a linked list */
  listContent(list_reply, replica_id);

  //parse list                                                                
  if(strcmp(list_reply,"EMPTY")==0){
    //writeLogFile("Empty!");
    return 0;
  }

  char delete_file[30][MAX_NAMELEN];
  int file_to_delete = 0;
  int i;
  int obj_num=0;
  int obj_field=0;
  int infield_index=0;
  for(i=0;i<2000;i++){
    char tmp_char = list_reply[i];
    if(tmp_char==')'){
      obj_num++;
      obj_field = 0;
      infield_index=0;
      if(list_reply[i+1]!='('){
	break;
      }
    }else if(tmp_char=='('){
      //initialize object                                                                   
      my_cache_list[obj_num] = malloc(sizeof(struct cache_index));
      memset(my_cache_list[obj_num]->name,0,MAX_NAMELEN+1);
      memset(my_cache_list[obj_num]->version,0,20);
      memset(my_cache_list[obj_num]->md5_hash,0,32);
    }else if(tmp_char==','){
      obj_field++;
      infield_index=0;
    }else{
      switch(obj_field){
      case 0:
	my_cache_list[obj_num]->name[infield_index] = tmp_char;
	break;
      case 1:
	my_cache_list[obj_num]->version[infield_index] = tmp_char;
	break;
      case 2:
	my_cache_list[obj_num]->md5_hash[infield_index] = tmp_char;
	break;
      }
      infield_index++;
    }

  }

  char acquire_list[2000];
  strcpy(acquire_list, "");


  //search newer version
  list_for_each (n, &entries) {
    struct ou_entry* o = list_entry(n, struct ou_entry, node);
    for(i=0; i<30; i++){
      if(my_cache_list[i]!=NULL && strcmp(my_cache_list[i]->name,o->name)==0){
	int db_timestamp = atoi(my_cache_list[i]->version);
	if(db_timestamp<=o->tv.tv_sec){
	  //ignore it
	}else{
	  //db has newer version,compare hash to decide whether to acquire it
	  if(strcmp(o->md5_hash,my_cache_list[i]->md5_hash)!=0){
	    strcat(acquire_list,o->name);
	    strcat(acquire_list,",");
	  }
	}
	free(my_cache_list[i]);
	my_cache_list[i] = NULL;
	break;
      }
    }
    //file not found, build delete list
    if(i==30&&strcmp(o->name,".")!=0&&strcmp(o->name,"..")!=0){
      strcpy(delete_file[file_to_delete],o->name);
      file_to_delete++;
    }
  }
  
  //delete file not exist in database
  for(i=0; i<file_to_delete; i++){
    list_for_each_safe (n, p, &entries) {
      struct ou_entry* o = list_entry(n, struct ou_entry, node);
      if (strcmp(delete_file[i], o->name) == 0) {
	__list_del(n);
	char fullpath[MAX_NAMELEN];
	fullPath(fullpath, o->name);
	res = unlink(fullpath);
	if(res==-1)
	  return -errno;
	free(o);
	break;
      }
    }
  }

  //search new file
  for(i=0; i<30; i++){
    if(my_cache_list[i]!=NULL){
      strcat(acquire_list, my_cache_list[i]->name);
      strcat(acquire_list,":");
      free(my_cache_list[i]);
      my_cache_list[i] = NULL;
    }
  }

  if(strcmp(acquire_list,"")==0){
    strcpy(acquire_list,"EMPTY");
  }
  
  char server_reply[2000];
  sendList(acquire_list,server_reply,replica_id);
  int retry = 0;
  while(strcmp(server_reply,"REQUEST_OK")!=0 && retry<MAX_RETRY){
    sendList(acquire_list, server_reply, replica_id);
  }

  return 0;
  

}

void update_md5(struct ou_entry* entry){
  char md5_data[1024];
  char md5_hex[33];
  unsigned char md5_digest[16];
  //sprintf(md5_data, "%s%d", entry->name, (int)entry->tv.tv_sec);
  MD5_CTX context;
  MD5_Init(&context);
  
  
  char full_path[MAX_NAMELEN];
  fullPath(full_path, entry->name);
  // sprintf(full_path,"/tmp/%s",entry->name);
  FILE* pFile = fopen(full_path, "r");
  if(pFile==NULL){
    return;
  }
  int bytes;
  while((bytes = fread(md5_data,1,1024,pFile))!=0){
    MD5_Update(&context, md5_data, bytes);
  }
  fclose(pFile);
  //MD5_Update(&context, md5_data, strlen(md5_data));
  MD5_Final(md5_digest, &context);
  
  char server_reply[100];
  int h;
  for(h=0;h<16;h++){
    sprintf(md5_hex+2*h,"%02x",md5_digest[h]);
  }

#ifdef MASTER
  postContent(entry->name,(int)entry->tv.tv_sec,md5_hex,server_reply);
  //error checking
  int retry=0;
  while(strcmp(server_reply,"POST_OK")!=0 && retry<MAX_RETRY){
    postContent(entry->name,(int)entry->tv.tv_sec,md5_hex,server_reply);
    retry++;
  }
#endif

  strcpy(entry->md5_hash,md5_hex);

  #ifdef DEBUG
  writeLogFile(entry->md5_hash);
  writeLogFile(server_reply);
  #endif
}


static int my_getattr(const char *path, struct stat *st)
{
  struct list_node* n;

  memset(st, 0, sizeof(struct stat));

  //read directory

  if (strcmp(path, "/") == 0) {
    st->st_mode = 0755 | S_IFDIR;
    st->st_nlink = 2;
    st->st_size = 0;

    list_for_each (n, &entries) {
      //struct ou_entry* o = list_entry(n, struct ou_entry, node);
      ++st->st_nlink;
      st->st_size += 1;
    }

    return 0;
  }
  
  //read regular file
  char whole_path[MAX_NAMELEN];
  fullPath(whole_path, path);
  int res;

  res = lstat(whole_path,st);
  if(res==-1)
    return -errno;
  /*list_for_each (n, &entries) {                                               
    struct ou_entry* o = list_entry(n, struct ou_entry, node);                 
    if (strcmp(path + 1, o->name) == 0) {                                      
      st->st_mode = o->mode;                                                   
      return 0;                                                                
    }                                                                          
    } */

  return 0;
}

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
  (void) offset;
  (void) fi;

#ifdef REPLICA
  getDBList();
#endif

  struct list_node* n;

  if (strcmp(path, "/") != 0)
    return -ENOENT;

  //filler(buf, ".", NULL, 0);
  //filler(buf, "..", NULL, 0);

  list_for_each (n, &entries) {
  struct ou_entry* o = list_entry(n, struct ou_entry, node);
    filler(buf, o->name, NULL, 0);
  }

  return 0;

}

static int my_mkdir(const char * path, mode_t mode)
{
    int ret = 0;
    char fpath[MAX_NAMELEN];
    printf("debug: mkdir(path=\"%s\", mode 0%3o)\n", path, mode);
    fullPath(fpath, path);

    ret = mkdir(fpath, mode);
    if (ret < 0)
    {
        ret = -errno;
        printf("error: mkdir mkdir\n");
    }

    return ret;

}


static int my_utimens(const char *path, const struct timespec ts[2])
{
  int res;
  char whole_path[MAX_NAMELEN];
  fullPath(whole_path, path);

#ifndef OSX
  res = utimensat(0, whole_path, ts, AT_SYMLINK_NOFOLLOW);
#else
  struct timeval tv[2];
  tv[0].tv_sec = ts[0].tv_sec;
  tv[0].tv_usec = ts[0].tv_nsec/1000;
  tv[1].tv_sec = ts[1].tv_sec;
  tv[1].tv_usec = ts[1].tv_nsec/1000;
  res = utimes(whole_path, tv);
#endif

  if (res == -1)
    return -errno;

  //maintain our own timestamp
  struct list_node* n;
  list_for_each (n, &entries) {
    struct ou_entry* o = list_entry(n, struct ou_entry, node);
    if (strcmp(path + 1, o->name) == 0) {
      //we only keep last modification time, ignore last access time
      o->tv.tv_sec = ts[1].tv_sec;
      o->tv.tv_nsec = ts[1].tv_nsec;
      update_md5(o);
      return 0;
    }
  }

  return -ENOENT;
}


static int my_chmod(const char * path, mode_t new_mode)
{
  int res;
  char whole_path[MAX_NAMELEN];
  fullPath(whole_path, path);

  res = chmod(whole_path, new_mode);
  if (res == -1)
    return -errno;

  struct list_node *n;

  //maintain our own mode
  list_for_each (n, &entries) {
    struct ou_entry* o = list_entry(n, struct ou_entry, node);
    if (strcmp(path + 1, o->name) == 0) {
      //we only keep last modification time, ignore last access time
      o->mode = new_mode;
      return 0;
    }
  }

  return -ENOENT;

}

static int my_chown(const char * path, uid_t uid, gid_t gid)
{
  int res;
  char whole_path[MAX_NAMELEN];
  fullPath(whole_path, path);

  res = chown(whole_path, uid, gid);
  if (res == -1)
    return -errno;

  return 0;
}





static int my_open(const char *path, struct fuse_file_info *fi)
{
  int res = 0;
  char whole_path[MAX_NAMELEN];
  fullPath(whole_path, path);

  res = open(whole_path, fi->flags);
  if(res==-1)
    return -errno;

  //fill in file handler
  fi->fh = res;

  return 0;
}

static int my_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{

  int res;

  res = pread(fi->fh, buf, size, offset);
  if (res == -1)
    res = -errno;
  close(fi->fh);
  return res;

}


static int my_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
  int res;

  res = pwrite(fi->fh, buf, size, offset);
  if (res == -1)
    res = -errno;
  close(fi->fh);

  struct list_node* n;
  list_for_each (n, &entries) {
    struct ou_entry* o = list_entry(n, struct ou_entry, node);
    if (strcmp(path + 1, o->name) == 0) {
      o->tv.tv_sec = time(NULL);
      update_md5(o);
      transfer_put(o->name);
      return res;
    }
  }



  return res;

}

static int my_truncate(const char *path, off_t size)
{
  int res;

  char whole_path[MAX_NAMELEN];
  fullPath(whole_path, path);

  res = truncate(whole_path, size);
  if (res == -1)
    return -errno;
  return 0;
}

static int my_rename(const char *from, const char *to)
{
  int res;
  res = rename(from, to);
  printf("this is rename from: %s, and goto : %s\n", from, to );
  transfer_get(from, to);
  if (res == -1)
    return -errno;

  return 0;
}


static int my_mknod(const char *path, mode_t mode, dev_t dev)
{
    int ret = 0;
    char fpath[MAX_NAMELEN];
    fullPath(fpath, path);

    // in linux, use mknod(path, mode, rdev),
    // this is generic implement, like osx
    #ifndef OSX
        ret = mknod(fpath, mode, dev);
    #else
        if (S_ISREG(mode))
        {
            ret = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
            if (ret < 0)
            {
                ret = -errno;
                printf("error: mknod open\n" );
            }else
            {
                ret = close(ret);
                if (ret < 0)
                {
                    ret = -errno;
                    printf("error: mknod close\n");
                }
            }
        }else if (S_ISFIFO(mode))
        {
            ret = mkfifo(fpath, mode);
            if (ret < 0)
            {
                ret = -errno;
                printf("error: mknod mkfifo\n");
            }else
            {
                ret = mknod(fpath, mode, dev);
                if (ret < 0)
                {
                    ret = -errno;
                    printf("error: mknod mknod\n");
                }
            }
        }
    #endif // OSX

    return ret;
}


static int my_create(const char* path, mode_t mode, struct fuse_file_info* fi)
{
  struct ou_entry* o;
  struct list_node* n;

  if (strlen(path + 1) > MAX_NAMELEN)
    return -ENAMETOOLONG;

  list_for_each (n, &entries) {
    o = list_entry(n, struct ou_entry, node);
    if (strcmp(path + 1, o->name) == 0)
      return -EEXIST;
  }

  o = malloc(sizeof(struct ou_entry));
  strcpy(o->name, path + 1); /* skip leading '/' */
  #ifdef MASTER
  o->mode = 0660 | S_IFREG;
  #endif
  #ifdef REPLICA
  o->mode = 0660 | S_IFREG;
  #endif

  list_add_prev(&o->node, &entries);

  char whole_path[MAX_NAMELEN];
  fullPath(whole_path, path);

  
  int res = 0;
  #ifdef MASTER
  res = creat(whole_path, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
  #endif
  #ifdef REPLICA
  res = creat(whole_path, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
  #endif

  if(res==-1){
    return -errno;
  }

  fi->fh = res;
  //writeLogFile("File Created!");

  return 0;
}

static int my_unlink(const char* path)
{
  struct list_node *n, *p;
  

  char whole_path[MAX_NAMELEN];
  fullPath(whole_path, path);
  int res;
  list_for_each_safe (n, p, &entries) {
    struct ou_entry* o = list_entry(n, struct ou_entry, node);
    if (strcmp(path + 1, o->name) == 0) {
      __list_del(n);
      res = unlink(whole_path);

      transfer_delete(o->name);

      if(res==-1)
	       return -errno;

      #ifdef MASTER
      //tell db to delete record
      char server_reply[100];
      delContent(o->name,server_reply);
      int retry=0;
      

      while(strcmp(server_reply,"DELETE_OK")!=0&&retry<MAX_RETRY){
	delContent(o->name,server_reply);
	retry++;
      }
      #endif

      free(o);
      return res;
    }
  }


  return -ENOENT;
}

int UpdateList(char* path){
  DIR* dp;
  struct dirent* de;

  dp = opendir(path);
  if(dp==NULL){
    return -errno;
  }

  struct ou_entry* o;
  
  while((de = readdir(dp))!=NULL){
    o = malloc(sizeof(struct ou_entry));
    strcpy(o->name,de->d_name);

    printf("%s",o->name);
    char full_path[MAX_NAMELEN];
    fullPath(full_path, o->name);
    // sprintf(full_path,"/tmp/%s",o->name);
    struct stat* stbuf = malloc(sizeof(struct stat));
    memset(stbuf, 0, sizeof(struct stat));
    lstat(full_path, stbuf);
    o->mode = stbuf->st_mode;
    o->tv.tv_sec = stbuf->st_mtime;
    free(stbuf);

    list_add_prev(&o->node, &entries);
  }

  closedir(dp);
  
  return 0;
}



static struct fuse_operations hello_oper = {
  .getattr= my_getattr,
  .readdir= my_readdir,
  .open= my_open,
  .read= my_read,
  .write= my_write,
  .create = my_create,
  .unlink = my_unlink,
  .truncate = my_truncate,
  .utimens = my_utimens,
  .rename = my_rename,
  .chmod = my_chmod,
  .chown = my_chown,
  .mknod = my_mknod,
  .mkdir = my_mkdir,
};




int main(int argc, char *argv[])
{
    
    #ifdef DEBUG
        log_file = fopen("/mylog/log","a");
    #endif

  list_init(&entries);
  UpdateList(ROOT_DIR);

#ifdef MASTER
    // socket_init();
    kick_start();

  
    int sock;
    pthread_t thread;
    struct addrinfo hints, *ret;
    int reuseaddr = 1; /* True */



    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(NULL, PORT, &hints, &ret) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    /* Create the socket */
    sock = socket(ret->ai_family, ret->ai_socktype, ret->ai_protocol);
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
    if (bind(sock, ret->ai_addr, ret->ai_addrlen) == -1) {
        perror("bind");
        return 0;
    }
    puts("bind done");

    freeaddrinfo(ret);

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
        size_t newsock = accept(sock, (socklen_t *)&their_addr, &size);
        if (newsock == -1) {
            perror("accept");
        }
        else {
            printf("Got a connection from %s on port %d\n", 
                    inet_ntoa(their_addr.sin_addr), htons(their_addr.sin_port));
            if (pthread_create(&thread, NULL, handle, &newsock) != 0) {
                fprintf(stderr, "Failed to create thread\n");
            }
            break;
        }
	}

#endif
  int res = fuse_main(argc, argv, &hello_oper, NULL);
  


  #ifdef DEBUG
  fclose(log_file);
  #endif

  return res;
}

