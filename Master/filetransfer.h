#include <stdio.h>
#include <string.h>

#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


#define USERNAME        "catgt"
#define PASSWD          "hackgt"


#ifdef MASTER

#define REMOTE_URL      "ftp://zeroc.at/tmp" // UPLOAD_FILE_AS
#define ROOT_DIR 		"/home/zechen/fuse"

#elif YY

#define REMOTE_URL      "ftp://localhost/remote_tmp" // UPLOAD_FILE_AS
#define ROOT_DIR 		"/tmp"

#elif REPLICA1

#define REMOTE_URL      "ftp://zeroc.at/cat1" // UPLOAD_FILE_AS
#define ROOT_DIR 		"/home/catgt/cat1"

#elif REPLICA2

#define REMOTE_URL      "ftp://zeroc.at/cat2" // UPLOAD_FILE_AS
#define ROOT_DIR 		"/home/catgt/cat1"

#endif


struct FtpFile
{
	const char *name;
	FILE *stream;	
};

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);


int transfer_put(const char * filename);


int transfer_get(const char *remote, const char * filename);
int transfer_delete(const char * filename);
