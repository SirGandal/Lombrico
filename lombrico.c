#include <stdio.h>
#include <string.h>
#include "rep.h"
#include "outputDblpParser.h"
#include "parserIEEE.h"
#include "parserACM.h"
#include "connector.h"

int main(int argc, char* argv[]) {

    char *url;
    char **urlsIeee;
    char choose;
    char contentWoAuthor;
    int i, numberOfIeeePages;
    referencesList refList = NULL;

    do {
        printf("\nThis operation will delete some of the files in the directory 'outputForDblp'.\nAre you sure that you want to continue?(y/n)");
        scanf("%c", &choose);
    } while(choose!='y' && choose!='n');

    if(choose=='n') {
        return 0;
    }

    if(argv[1]) {
        downloadFromUrl(argv[1]);
    } else {
        printf("Error - You've to pass the url of the website to download.\n");
        return 0;
    }

    /*
     *Checking which site are we talkin about to call the corresponding
     *parser function
     */

    if(strstr(argv[1],"http://ieeexplore.ieee.org/")!=NULL) {

        printf("%s\n", "Type: IEEE");

        do {
            printf("\nDo you want to insert the content without authors in the final html page?(y/n)");
            scanf("%*c %c", &contentWoAuthor);
        } while(contentWoAuthor!='y' && contentWoAuthor!='n');

         printf("Parsing the web page.\n");

        urlsIeee = firstParserIEEE(argv[1]);

        for(numberOfIeeePages=0; strcmp(urlsIeee[numberOfIeeePages],"end")!=0; numberOfIeeePages++);

        printf("Found %d pages.\n",numberOfIeeePages);

        for(i=0; i<numberOfIeeePages; i++) {

            //printf("URL %d: %s\n", i+1, urlsIeee[i]);

            printf("Downloading web page number %d.\n",i+1);

            downloadFromUrl(urlsIeee[i]);

            printf("Parsing web page number %d.\n",i+1);

            secondParserIEEE(&refList);

            printf("Building the HTML page 'outputForDblp-%d.html'.\n",i+1);

            outputDblpParser(refList,numberOfIeeePages,i+1,contentWoAuthor);

            refList = NULL;
        }

        //Removing the temporary file used only for parsing
        unlink("tmp.txt");

    } else {

        if(strstr(argv[1],"http://portal.acm.org/")!=NULL) {

            printf("%s\n", "\nType: ACM");

            /*
             *url contains the url of the table of contents for the site in argument argv[1]
             */
            printf("Parsing the web page.\n");

            url = firstParserACM();

            //printf("Url di TOCs:\n%s", url);

            printf("Downloading the table of contents.\n");

            downloadFromUrl(url);

            printf("Parsing the table of contents.\n");

            secondParserAcm(&refList);

            printf("Building the HTML page.\n");

            outputDblpParser(refList,1,1,'n');

            //Removing the temporary file used only for parsing
            unlink("tmp.txt");

        }

    }

    printf("\nCheck out the output in the 'outputForDblp' directory.\n\n");

    return 0;
}
