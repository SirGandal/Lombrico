#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#define MYFILE "tmp.txt"

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {

    size_t written;

    written = fwrite(ptr,size,nmemb,stream);

    return written;
}

/*!
 *This function download the page that is related to the url passed in argument.
 *
 *\param url
 *          Url of the web page that has to be downloaded.
 *\return The pointer of the file created. The file 'tmp.txt' will contains the html of the web page.
 */
FILE * downloadFromUrl(char* url) {
    CURL *curl;
    CURLcode res;
    FILE *fp;

    char outfilename[FILENAME_MAX] = MYFILE;
    fp = fopen(outfilename,"wb");

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);
        fclose(fp);

        if(res == 0) {
            printf("%s\n", "Website downloaded correctly.");
        } else {
            printf("\n%s '%s'", "Failure in downloading website at url",url);
        }

        curl_easy_cleanup(curl);
    }
    return fp;
}
