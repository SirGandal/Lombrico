#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <curl/curl.h>
#include "rep.h"
#include "pcre.h"
#include "outputDblpParser.h"

#define URLACM "http://portal.acm.org/"
#define FILEACM "tmp.txt"
#define OVECCOUNT 30

/*!
 *This function is used to analyze the site passed as an argument from command line.
 *The site has been previously download by the function 'downloadFromUrl' in 'connection.c'.
 *This parser read from the file 'tmp.txt' previously downloaded containing the html of
 *the site and then catch the link of the site that has to be downloaded where is written
 *the table of contents.
 *
 *\return The URL corresponding to the Table of contents.
 */
char * firstParserACM() {
    int beginIndex, endIndex; //beginIndex end endIndex will contain the index of the URL's start and URL's end
    int erroffset;
    int ovector[OVECCOUNT];
    int rc;
    int i = 0;
    int z = 0;
    long int fileLength = 0;
    FILE *fp = NULL;
    char* support = NULL;
    pcre *re;
    const char *error;
    char *regex = NULL;
    char *dataReg = NULL;
    char *returnURL;

    fp = fopen(FILEACM, "r+");
    if(fp == NULL ) {
        perror("\nError in opening file");
        return;
    }

    fseek(fp,0,SEEK_END);
    fileLength = ftello(fp);
    fseek(fp,0,SEEK_SET);

    support = (char*)malloc(fileLength*sizeof(char));
    if(support == NULL) {
        perror("\nError in allocating memory");

        return;
    }

    /*
     *Copying alle the characters in tmp.txt in a support array used to parse what's needed.
     */
    fscanf(fp, "%c", support+i);
    while(!feof(fp)) {
        i++;
        fscanf(fp,"%c", support+i);
    }

    /*
     *The link that we are searching is the only one in the support array (generally speaking in the website)
     *that begin in this way followed by lots of various characters and number.
     */
    regex = "tab_about.cfm\?([^'][^]])+";
    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! re) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return;
    }

    rc = pcre_exec(re, NULL, support, strlen(support),0, 0, ovector, OVECCOUNT);
    beginIndex=ovector[0];

    /*
     *The link that start with the previous regex end with this one.
     */
    regex = "']}";
    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! re) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return;
    }

    rc = pcre_exec(re, NULL, support, strlen(support),beginIndex, 0, ovector, OVECCOUNT);
    endIndex=ovector[0];

    /*
     *Building the URL to return,
     */
    returnURL = (char *)malloc(((endIndex-beginIndex)+1+strlen(URLACM))*(sizeof(char)));
    strcpy(returnURL, URLACM);

    for(i=beginIndex,z=strlen(URLACM); i<endIndex; i++,z++) {
        returnURL[z]=support[i];
    }
    returnURL[z]='\0';

    return returnURL;
}

/*!
 *This function is used to analyze the table of contents.
 *Parsing is made on the file 'tmp.txt' corresponding to the table of contents previously
 *downloaded by the function 'downloadFromUrl' in 'connection.c'.
 *
 *\param refList
 *              List that have to be filled with all checked papers.
 */
void secondParserAcm(referencesList *refList) {
    //These are all the index that are used to match every section in a content
    int index, titleIndex, titleBeginIndex, titleEndIndex, authorIndex, authorBeginIndex, authorEndIndex, pagesIndex, firstPagesIndex, secondPagesIndex, eeIndex, eeBeginIndex, eeEndIndex, spanIndex;
    int firstOccurence, count, currentNumber;
    int i = 0;
    int z = 0;
    long int fileLength = 0;
    FILE *fp = NULL;
    char* support = NULL;
    pcre *re, *masterRe, *reTitle, *reBeginAuthor, *reEndAuthors, *reAuthor, *rePageBegin, *rePageIntermediate, *rePageEnd, *reUrlBegin, *reUrlStart, *reUrlEnd;
    const char *error;
    int erroffset;
    int ovector[OVECCOUNT];
    int rc;
    char *regex = NULL;
    char *dataReg = NULL;
    char *title, *author, *pages, *url;

    authorsList autList = NULL;

    fp = fopen(FILEACM, "r+");
    if(fp == NULL ) {
        perror("\nError in opening file");
        return;
    }

    fseek(fp,0,SEEK_END);
    fileLength = ftello(fp);
    fseek(fp,0,SEEK_SET);

    support = (char*)malloc(fileLength*sizeof(char));
    if(support == NULL) {
        perror("\nError in allocating memory");
        return;
    }

    /*
     *Copying alle the characters in tmp.txt in a support array used to parse what's needed.
     */
    fscanf(fp, "%c", support+i);
    while(!feof(fp)) {
        i++;
        fscanf(fp,"%c", support+i);
    }

    for(i=0; i<OVECCOUNT; i++) {
        ovector[i]=-1;
    }

    /*What is interesting for us begin after the first </div>*/
    regex = "</div>";
    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! re) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    rc = pcre_exec(re, NULL, support, strlen(support),0, 0, ovector, OVECCOUNT);

    index=ovector[0];

    /*Here begin the count of the citation in the website.
     *This is useful to set a big While in order to pick up
     *automatically every citation.
     */
    regex = "citation\\.cfm";
    masterRe = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! re) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    rc = pcre_exec(masterRe, NULL, support, strlen(support),index, 0, ovector, OVECCOUNT);
    firstOccurence = ovector[1];

    /*
     *If the regex matches, rc will be >0 and so the 'count' of citation is initialized at 1.
     */
    if(rc>0){
        count=1;
    }else{
        count=0;
    }

    /*
     *Counting the number of citation.
     */

    //The count will begin from the firstOccurence
    i=firstOccurence;

    while(1) {
        rc = pcre_exec(masterRe, NULL, support, strlen(support),i, 0, ovector, OVECCOUNT);
        i=ovector[1];

        /*
         *The pcre_exec search in the support array continously.
         *If the index of the current regex that matches is previous to the first
         *occurence of the citation the cycle will end.
         */
        if(i<firstOccurence) {
            break;
        } else {
            count++;
        }
    }

    /*In count there will be the total number of contents.
     *So from now on I set a while with a counter of the number
     *of reference yet processed. When the number of reference yet
     *processed is equal to to count (let's say it again) the total
     *number of all references the while stop and the list of
     *references is passed to the final parser that build th html output.
     */

    rc = pcre_exec(masterRe, NULL, support, strlen(support),index, 0, ovector, OVECCOUNT);

    titleIndex=ovector[0];

    if(rc) {
        currentNumber=1;
    } else {
        currentNumber=0;
    }

    /*
     *From now on are compiled the regular expressions that will be used
     *later during the parse of the page.
     */

    regex = ">[^<]+<";
    reTitle = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! reTitle) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    regex = "author_page.cfm?";
    reBeginAuthor = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! reBeginAuthor) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    /*
     *Every block of authors is ended by a </span>.
     */
    regex = "</span>";
    reEndAuthors = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! reEndAuthors) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    regex = ">([a-zA-Z\\-&#0-9;][ .]?[ ]?)+<";
    reAuthor = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! reAuthor) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    regex = "Page.?:";
    rePageBegin = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! rePageBegin) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    regex = "<";
    rePageEnd = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! rePageEnd) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    regex = "Full text:|Available formats:|Full text available:";
    reUrlBegin = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! reUrlBegin) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    regex = "ft_gateway.cfm?";
    reUrlStart = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! reUrlStart) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    regex = "\"";
    reUrlEnd = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! reUrlEnd) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
    }

    while(currentNumber <= count) {
        //printf("\nReferenza numero %d\n",currentNumber);

        rc = pcre_exec(reTitle, NULL, support, strlen(support),titleIndex, 0, ovector, OVECCOUNT);

        titleBeginIndex=ovector[0]+1;
        titleEndIndex=ovector[1]-2;

        title = (char *)malloc((titleEndIndex-titleBeginIndex+2)*(sizeof(char)));

        for(i=titleBeginIndex,z=0; i<=titleEndIndex; i++,z++) {
            title[z]=support[i];
        }
        title[z]='\0';

        rc = pcre_exec(reBeginAuthor, NULL, support, strlen(support),titleEndIndex, 0, ovector, OVECCOUNT);
        authorIndex=ovector[0];

        rc = pcre_exec(reEndAuthors, NULL, support, strlen(support),authorIndex, 0, ovector, OVECCOUNT);
        spanIndex=ovector[0];

        rc = pcre_exec(reAuthor, NULL, support, strlen(support),authorIndex, 0, ovector, OVECCOUNT);

        authorBeginIndex=ovector[0]+1;
        authorEndIndex=ovector[1]-2;

        while(spanIndex > authorBeginIndex) {

            author = (char *)malloc((authorEndIndex-authorBeginIndex+2)*(sizeof(char)));
            for(i=authorBeginIndex,z=0; i<=authorEndIndex; i++,z++) {
                author[z]=support[i];
            }
            author[z]='\0';

            //printf("Autore:%s\n",author);
            //fflush(stdout);
            insertAuthorsInTail(&autList, author);

            rc = pcre_exec(reAuthor, NULL, support, strlen(support),authorBeginIndex, 0, ovector, OVECCOUNT);

            authorBeginIndex=ovector[0]+1;
            authorEndIndex=ovector[1]-2;
        }

        rc = pcre_exec(rePageBegin, NULL, support, strlen(support),titleEndIndex, 0, ovector, OVECCOUNT);

        firstPagesIndex=ovector[1]+1;

        rc = pcre_exec(rePageEnd, NULL, support, strlen(support),firstPagesIndex, 0, ovector, OVECCOUNT);

        secondPagesIndex=ovector[0]-1;

        if(secondPagesIndex==1 && firstPagesIndex==0) {
            pages="0-";
        } else {
            pages = (char *)malloc((secondPagesIndex-(firstPagesIndex)+2)*(sizeof(char)));

            for(i=firstPagesIndex,z=0; i<=secondPagesIndex; i++,z++) {
                pages[z]=support[i];
            }
            pages[z]='\0';
        }

        rc = pcre_exec(reUrlBegin, NULL, support, strlen(support),secondPagesIndex, 0, ovector, OVECCOUNT);

        /*
         *If the regex match there will be build the url of the eletronic edition.
         */
        if(rc!=-1) {
            eeIndex=ovector[0];

            rc = pcre_exec(reUrlStart, NULL, support, strlen(support),eeIndex, 0, ovector, OVECCOUNT);

            eeBeginIndex=ovector[0];

            rc = pcre_exec(reUrlEnd, NULL, support, strlen(support),eeBeginIndex, 0, ovector, OVECCOUNT);

            eeEndIndex=ovector[0]-1;

            url = (char *)malloc(((eeEndIndex-eeBeginIndex)+strlen(URLACM)+1)*(sizeof(char)));

            strcpy(url, URLACM);

            for(i=eeBeginIndex,z=strlen(URLACM); i<eeEndIndex; i++,z++) {
                url[z]=support[i];
            }
            url[z]='\0';
        } else {
            //By default if there is no eletronic resource the URL will be "".
            url="";
        }
        rc = pcre_exec(masterRe, NULL, support, strlen(support),titleIndex+1, 0, ovector, OVECCOUNT);

        titleIndex=ovector[0];

        currentNumber++;

        insertReferencesInTail(refList, pages, autList, title, url);

        autList = NULL;

    }
}
