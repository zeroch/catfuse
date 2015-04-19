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


#include "list.h"
#include "testdb.h"
#include "md5.h"

#define MAX_NAMELEN 255
#define DEBUG

extern void MD5_Init(MD5_CTX *ctx);
extern void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
extern void MD5_Final(unsigned char *result, MD5_CTX *ctx);

FILE* log_file;

struct ou_entry {
  mode_t mode;
  struct timespec tv;
  unsigned char md5_hash[16];
  struct list_node node;
  char name[MAX_NAMELEN + 1];
};

void writeLogFile(char* data){
  #ifdef DEBUG

  fprintf(log_file, "%s\n", data);

  #endif
}


void update_md5(struct ou_entry* entry){
  char md5_data[MAX_NAMELEN];
  sprintf(md5_data, "%s%d", entry->name, (int)entry->tv.tv_sec);
  MD5_CTX context;
  MD5_Init(&context);
  MD5_Update(&context, md5_data, strlen(md5_data));
  MD5_Final(entry->md5_hash, &context);
  
  char server_reply[100];
  char hexdigest[32];
  int h;
  for(h=0;h<16;h++){
    sprintf(hexdigest+h,"%02x",entry->md5_hash[h]);
  }
  postContent(entry->name,(int)entry->tv.tv_sec,hexdigest,server_reply);

  #ifdef DEBUG
  char log_msg[100];
  int i;
  for(i=0;i<16;i++){
    sprintf(log_msg+i,"%x",entry->md5_hash[i]);
  }
  writeLogFile(log_msg);
  writeLogFile(server_reply);
  #endif
}

  
static void fullPath(char fpath[MAX_NAMELEN], const char * path)
{
    strcpy(fpath, "/tmp");
    strncat(fpath, path, MAX_NAMELEN);
}

static struct list_node entries;

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
      struct ou_entry* o = list_entry(n, struct ou_entry, node);
      ++st->st_nlink;
      st->st_size += strlen(o->name);
    }

    return 0;
  }

  //read regular file
  char whole_path[MAX_NAMELEN];
  sprintf(whole_path,"/tmp%s",path);
  int res;
  list_for_each (n, &entries) {
    struct ou_entry* o = list_entry(n, struct ou_entry, node);
    if (strcmp(path + 1, o->name) == 0) {
      res = lstat(whole_path, st);
      if(res==-1)
	return -errno;
      st->st_mode = o->mode;
      st->st_nlink = 1;
      return 0;
    }
  }

  return -ENOENT;
}

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
  (void) offset;
  (void) fi;

  struct list_node* n;

  if (strcmp(path, "/") != 0)
    return -ENOENT;

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);

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
  sprintf(whole_path,"/tmp%s",path);

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
  sprintf(whole_path,"/tmp%s",path);

  res = chmod(whole_path, new_mode);
  if (res == -1)
    return -errno;

  //maintain our own mode
  struct list_node* n;
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
  sprintf(whole_path,"/tmp%s",path);

  res = chown(whole_path, uid, gid);
  if (res == -1)
    return -errno;

  return 0;
}





static int my_open(const char *path, struct fuse_file_info *fi)
{
  int res = 0;
  char whole_path[MAX_NAMELEN];
  sprintf(whole_path,"/tmp%s",path);

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
      return res;
    }
  }


  return res;

}

static int my_truncate(const char *path, off_t size)
{
  int res;

  char whole_path[MAX_NAMELEN];
  sprintf(whole_path,"/tmp%s",path);

  res = truncate(whole_path, size);
  if (res == -1)
    return -errno;
  return 0;
}

static int my_rename(const char *from, const char *to)
{
  int res;
  res = rename(from, to);
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
  o->mode = mode | S_IFREG;
  list_add_prev(&o->node, &entries);

  char whole_path[MAX_NAMELEN];
  sprintf(whole_path,"/tmp%s",path);

  int res = creat(whole_path, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
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
  sprintf(whole_path,"/tmp%s",path);
  int res;
  list_for_each_safe (n, p, &entries) {
    struct ou_entry* o = list_entry(n, struct ou_entry, node);
    if (strcmp(path + 1, o->name) == 0) {
      __list_del(n);
      free(o);
      res = unlink(whole_path);
      if(res==-1)
	return -errno;
      return res;
    }
  }


  return -ENOENT;
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
  log_file = fopen("/tmp/__my__log__","a");
  #endif
  list_init(&entries);
  int res = fuse_main(argc, argv, &hello_oper, NULL);
  
  #ifdef DEBUG
  fclose(log_file);
  #endif

  return res;
}

