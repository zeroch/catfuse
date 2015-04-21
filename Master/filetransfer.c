
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



size_t write_callback(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct FtpFile *out = (struct FtpFile *)stream;
    if (out && !out->stream)
    {
        out->stream=fopen(out->name, "wb");
        if (!out->stream)
        {
            return -1;
        }
    }

    curl_off_t nwrite;

    size_t retcode = fwrite(buffer, size, nmemb, out->stream);

    nwrite = (curl_off_t)retcode;
    
    fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n",  nwrite);

    return retcode;

}


int transfer_put(const char * remote, const char * filename)
{
    CURL *curl;
    CURLcode res;
    FILE *hd_src;
    struct stat file_info;
    curl_off_t fsize;


    /* get the file size of the local file */
    char fpath[256];
    strcpy(fpath, ROOT_DIR);
    strcat(fpath,"/");
    strncat(fpath, filename, 256);
    printf("this local file is %s\n", fpath);
    
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

    /* get a curl handle */
    curl = curl_easy_init();


  if(curl) {

        /* we want to use our own read function */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

        /* enable uploading */
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        char target_url[256];
        strcpy(target_url, remote);
        strcat(target_url, "/");
        strncat(target_url, filename, 256);

        /* specify target */
        curl_easy_setopt(curl,CURLOPT_URL, target_url);


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
        return 0;
    }

    fclose(hd_src); /* close the local file */
    return -1;

}


int transfer_get(const char *remote, const char * filename)
{

    CURL *curl;
    CURLcode res;
    char local_name[256];
    strcpy(local_name, ROOT_DIR);
    strcat(local_name, "/");
    strncat(local_name, filename, 256);

    struct FtpFile remote_file = {
        local_name, 
        NULL
    };

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl)
    {
        char src_url[256];
        strcpy(src_url, remote);
        strncat(src_url, filename, 256);

        curl_easy_setopt(curl, CURLOPT_URL, src_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remote_file);

        curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWD);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl failes %s\n", curl_easy_strerror(res));
        }

        if (remote_file.stream)
            fclose(remote_file.stream);

        return 0;
    }

    return -1;

}


int transfer_delete(const char * filename)
{
    CURL *curl;
    CURLcode res;


    #define DEL_CMD "DELE "
    #define CD_CMD "CD "

    struct curl_slist *headerlist=NULL;
    char buf_1 [256];
    // char buf_2[256];

    strcpy(buf_1, DEL_CMD);
    strncat(buf_1, filename, 256);
    printf("the current DELE command is %s\n", buf_1);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl)
    {
        
        // headerlist = curl_slist_append(headerlist, buf_2);
        headerlist = curl_slist_append(headerlist, buf_1);


        char src_url[256];
        strcpy(src_url, REMOTE_URL);
        strcat(src_url, "/");
        strncat(src_url, filename, 256);
        curl_easy_setopt(curl, CURLOPT_URL, src_url);
        /* READ FILE IS NULL, THIS IS ONLY HELP US  GET INTO THE DIR WE WANT */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);

        /* enable uploading, but dont pass anything  */
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        // printf("the current remote_root URL is %s\n", filename);
        // printf("the current filename URL is %s\n", filename);
        // printf("the current UPLOAD URL is %s\n", src_url);
  
        curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWD);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

        curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all (headerlist);

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl failes %s\n", curl_easy_strerror(res));
        }

        return 0;
    }

    return -1;
}