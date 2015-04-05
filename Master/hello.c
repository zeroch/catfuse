/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "list.h"

#define MAX_NAMELEN 255

struct ou_entry {
  mode_t mode;
  struct list_node node;
  char name[MAX_NAMELEN + 1];
};
 
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

static int my_open(const char *path, struct fuse_file_info *fi)
{
  int res;
  char whole_path[MAX_NAMELEN];
  sprintf(whole_path,"/tmp%s",path);

  res = open(whole_path, fi->flags);
  if(res==-1)
    return -errno;

  close(res);

  return 0;
}

static int my_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
  int fd;
  int res;
  (void) fi;
  char whole_path[MAX_NAMELEN];
  sprintf(whole_path,"/tmp%s",path);

  fd = open(whole_path, O_RDONLY);
  if (fd == -1)
    return -errno;
  res = pread(fd, buf, size, offset);
  if (res == -1)
    res = -errno;
  close(fd);
  return res;

}


static int my_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
  int fd;
  int res;
  (void) fi;
  char whole_path[MAX_NAMELEN];
  sprintf(whole_path,"/tmp%s",path);

  fd = open(whole_path, O_WRONLY);
  if (fd == -1)
    return -errno;
  res = pwrite(fd, buf, size, offset);
  if (res == -1)
    res = -errno;
  close(fd);

  return res;

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
  close(res);

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
};

int main(int argc, char *argv[])
{
  list_init(&entries);
  return fuse_main(argc, argv, &hello_oper, NULL);
}

