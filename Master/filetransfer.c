
#include "filetransfer.h"

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  curl_off_t nread;
  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  size_t retcode = fread(ptr, size, nmemb, stream);

  nread = (curl_off_t)retcode;

  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n", nread);
  return retcode;
}


int transfer_init(const char * filename)
{
  CURL *curl;
  CURLcode res;
  FILE *hd_src;
  struct stat file_info;
  curl_off_t fsize;

  // struct curl_slist *headerlist=NULL;
  // static const char buf_1 [] = "RNFR " UPLOAD_FILE_AS;
  // static const char buf_2 [] = "RNTO " RENAME_FILE_TO;

  /* get the file size of the local file */
  char fpath[256];
    strcpy(fpath, ROOT_DIR);
    strcat(fpath,"/");
    strncat(fpath, filename, 256);

  if(stat(fpath, &file_info)) {
    printf("Couldnt open '%s': %s\n", fpath, strerror(errno));
    return 1;
  }
  fsize = (curl_off_t)file_info.st_size;

  printf("Local file size: %" CURL_FORMAT_CURL_OFF_T " bytes.\n", fsize);

  /* get a FILE * of the same file */
  hd_src = fopen(fpath, "rb");

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  printf("after global init\n");
  /* get a curl handle */
  curl = curl_easy_init();
  printf("after easy init\n");
  if(curl) {
    /* build a list of commands to pass to libcurl */
    // headerlist = curl_slist_append(headerlist, buf_1);
    // headerlist = curl_slist_append(headerlist, buf_2);

    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);


    char target_url[256];
    strcpy(target_url, REMOTE_URL);
    strcat(target_url, "/");
    strncat(target_url, filename, 256);
    printf("the current remote_root URL is %s\n", filename);
    printf("the current filename URL is %s\n", filename);
    printf("the current UPLOAD URL is %s\n", target_url);
    /* specify target */
    curl_easy_setopt(curl,CURLOPT_URL, target_url);

    /* pass in that last of FTP commands to run after the transfer */
    // curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

    /* set up the username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWD);

    /* Set the size of the file to upload (optional).  If you give a *_LARGE
       option you MUST make sure that the type of the passed-in argument is a
       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
       make sure that to pass in a type 'long' argument. */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)fsize);
    printf("before easy curl_easy_perform\n" );
    /* Now run off and do what you've been told! */
    res = curl_easy_perform(curl);
    printf("after easy curl_easy_perform\n" );
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* clean up the FTP commands list */
    // curl_slist_free_all (headerlist);
    printf("after curl_slist_free_all\n" );

    /* always cleanup */
    printf("before curl_easy_cleanup\n" );

    curl_easy_cleanup(curl);
  }
  fclose(hd_src); /* close the local file */
  return 0;
}