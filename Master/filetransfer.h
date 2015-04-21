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

#define REMOTE_URL      "ftp://zeroc.at/tmp" // UPLOAD_FILE_AS
#define ROOT_DIR "/home/zero/fuse"

struct FtpFile
{
	const char *name;
	FILE *stream;	
};

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);


int transfer_put(const char * filename);


int transfer_get(const char *remote, const char * filename);
int transfer_delete(const char * filename);
