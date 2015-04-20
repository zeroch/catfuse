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
#define ROOT_DIR "/home/zechen/fuse"


size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);


int transfer_init(const char * filename);
