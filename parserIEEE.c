#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <curl/curl.h>
#include "pcre.h"
#include "rep.h"
#define URLIEEE "http://ieeexplore.ieee.org"
#define OVECCOUNT 30
#define FILEIEEE "tmp.txt"
#define FIRSTPARTURL "http://ieeexplore.ieee.org/xpl/mostRecentIssue.jsp?asf_arn=null&asf_iid=null&asf_pun="
#define SECONDPARTURL "&asf_in=null&asf_rpp=null&asf_iv=null&asf_sp=null&asf_pn="

int counterPaper(char*data);
char* splitTitle(char* data);
authorsList splitAuthors(char* data);
char* splitPP(char* data, int);

/*!
 *The function 'firstParserIEEE' checks if there are 1 or more pages in current conference.
 *If we are in a multi-pages conference the function builds URL's of other pages.
 *
 *\param url
 *          The main page url of the conference.
 *
 *\return An array of page links.
 */
char** firstParserIEEE(char *url) {
    int beginIndex, endIndex;
    int i = 0;
    int z = 0;
    int count = 1; //because of the current page
    long int fileLength = 0;
    FILE *fp = NULL;
    char* support = NULL;
    pcre *re;
    const char *error;
    int erroffset;
    int ovector[OVECCOUNT];
    int rc;
    char *regex = NULL;
    char *dataReg = NULL;
    char *numberUrl;
    char **urls;
    int urlLength;
    char indexPageNumber[4];

    fp = fopen(FILEIEEE, "r+");
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

    fscanf(fp, "%c", support+i);
    while(!feof(fp)) {
        i++;
        fscanf(fp,"%c", support+i);
    }
    regex = "punumber=[0-9]+";
    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! re) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return;
    }

    rc = pcre_exec(re, NULL, support, strlen(support),0, 0, ovector, OVECCOUNT);
    numberUrl = (char *)malloc((ovector[1]+9-ovector[0]+2)*(sizeof(char)));

    for(i=ovector[0]+9,z=0; i<ovector[1]; i++,z++) {
        numberUrl[z]=support[i];
    }
    numberUrl[z]='\0';

    regex = "javascript:gotoPage";
    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! re) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return;
    }

    rc = pcre_exec(re, NULL, support, strlen(support),0, 0, ovector, OVECCOUNT);
    beginIndex=ovector[1];

    while(rc>0) {
        count++;
        rc = pcre_exec(re, NULL, support, strlen(support),beginIndex, 0, ovector, OVECCOUNT);
        beginIndex=ovector[1];
    }

    urlLength = strlen(FIRSTPARTURL)+strlen(numberUrl)+strlen(SECONDPARTURL)+3;
    urls = (char **)malloc((count+1)*(sizeof(char *)));

    for(i=0; i<count; i++) {
        urls[i] = (char*)malloc((urlLength+1)*sizeof(char));
    }
    for(i=0; i<count; i++) {
        urls[i][0] = '\0';

        strcat(urls[i],FIRSTPARTURL);
        strcat(urls[i],numberUrl);
        strcat(urls[i],SECONDPARTURL);
        sprintf(indexPageNumber, "%d", (i+1));
        strcat(urls[i],indexPageNumber);
    }

    /*
     *I put "end" as last url in the array in order to know,
     *when i call a for cicle on this array, when to end the cycle.
     */
    urls[i] = "end";
    return urls;
}
/*!
 *The function 'secondParserIEEE' finds first start and end of conference part. Then, it cycles for a calculeted number of time
 *and parses for each paper of conference (if available) title, author, pages and electronic version of paper. Finally it
 *inserts in a list ('refList')
 *
 *\param refList
 *              List that have to be filled with all checked papers.
 *
 *\return 1 if it terminated with a success. Number of error in other case: 404, file non found; -1 Regex non compilable;
 *-2 Allocation of memory error.
 */
int secondParserIEEE(referencesList *refList) {
    int i = 0;
    int j = 0;
    int z = 0;
    int flag = 0;
    int counter = 0;
    int checkedPap = 0;
    int rc = 0;
    int erroffset = 0;
    int beginIndex = 0;
    int endIndex = 0;
    int h3Index = 0;
    int pagesIndex = 0;
    int h3EndIndex = 0;
    int specialAIndex = 0;
    int oabReIndex = 0;
    int sizeString = 0;
    int startUrl = 0;
    int endUrl = 0;
    long int fileLength = 0;
    FILE *fp = NULL;
    char* support = NULL;
    char* references = NULL;
    char* pages = NULL;
    char* title = NULL;
    pcre *re;
    pcre *hre;
    pcre *endHRe;
    pcre *pagesRe;
    pcre *oabRe;
    pcre *pdfRe;
    pcre *aEndRe;
    const char *error;
    int ovector[OVECCOUNT];

    authorsList alList = NULL;
    char *regex = NULL;
    char *h3Regex = NULL;
    char *h3EndRegex = NULL;
    char* pagesRegex = NULL;
    char* openAngleBracketRegex = NULL;
    char *specialRegex = "<b>";
    char *pdfRegex;
    char *aEndRegex;
    char *dataTemp = NULL;


    for(j = 0; j < OVECCOUNT; j++) {
        ovector[j] = -1;
    }

    fp = fopen(FILEIEEE, "r+");
    if(fp == NULL ) {
        perror("\nError in opening file");
        return 404;
    }
    fseek(fp,0,SEEK_END);
    fileLength = ftello(fp);
    fseek(fp,0,SEEK_SET);

    support = (char*)malloc(fileLength*sizeof(char));
    if(support == NULL) {
        perror("\nError in allocating memory");
        return -2;
    }

    fscanf(fp, "%c", support+i);
    while(!feof(fp)) {
        i++;
        fscanf(fp,"%c", support+i);
    }

    regex = "<!-- CODE FOR CHECKING ABSTRACTPLUS FOR USERS WHO HAVE VIRTUAL JOURNAL PACKAGES -->";
    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! re) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return -1;
    }
    rc = pcre_exec(re, NULL, support, strlen(support),0, 0, ovector, OVECCOUNT);
    beginIndex = ovector[0];

    regex = "<!-- END MAIN CONTENT -->";
    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (! re) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return -1;
    }
    rc = pcre_exec(re, NULL, support, strlen(support),beginIndex, 0, ovector, OVECCOUNT);
    endIndex = ovector[0];

    /*References array  contains just part concerning to 'References', between word 'CODE FOR CHECKING ABSTRACTPLUS...'
     and 'END MAINCONTENT'*/
    references = (char*)malloc( (endIndex - beginIndex +3)*sizeof(char));
    memcpy(references, &support[beginIndex], endIndex - beginIndex + 1);
    references[endIndex - beginIndex + 1 ] = '\0';

    counter = counterPaper(references);

    h3Regex = "<h3>";
    hre = pcre_compile(h3Regex, 0, &error,&erroffset,NULL);
    if (! hre) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return -1;
    }
    rc = pcre_exec(hre, NULL, references, strlen(references),0, 0, ovector, OVECCOUNT);
    h3Index = ovector[1];

    h3EndRegex = "</h3>";
    endHRe = pcre_compile(h3EndRegex, 0, &error,&erroffset,NULL);
    if (!endHRe) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return -1;
    }

    pagesRegex = "<b>Page\\(s\\):</b>";
    pagesRe = pcre_compile(pagesRegex, 0, &error,&erroffset,NULL);
    if (!pagesRe) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return -1;
    }

    openAngleBracketRegex = "<";
    oabRe = pcre_compile(openAngleBracketRegex, 0, &error,&erroffset,NULL);
    if (!oabRe) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return -1;
    }

    pdfRegex = "Full Text: <a href=";
    pdfRe = pcre_compile(pdfRegex, 0, &error,&erroffset,NULL);
    if (!pdfRe) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return -1;
    }

    aEndRegex = "\">PDF</a>";
    aEndRe = pcre_compile(aEndRegex, 0, &error,&erroffset,NULL);
    if (!aEndRe) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        return -1;
    }

    while(checkedPap < counter) {
        /*This cycles while number of paper cycled is less than all number of papers*/
        rc = pcre_exec(endHRe, NULL, references, strlen(references),h3Index, 0, ovector, OVECCOUNT);
        h3EndIndex = ovector[1];

        sizeString = h3EndIndex - h3Index;
        dataTemp = (char*)malloc((sizeString+3)*sizeof(char));

        memcpy(dataTemp, &references[h3Index], sizeString);

        /*It need a 'sizestring - 5' because we are at the end of '</h3> and we need to go to the start*/
        for(i = 0; dataTemp[i] == ' ' || dataTemp[i] == '\t' || dataTemp[i] == '\n'; i++);
        for(z = sizeString-5; dataTemp[z] == ' ' || dataTemp[z] == '\t' || dataTemp[z] == '\n'; z--);
        memmove(dataTemp, &dataTemp[i], z-i+1);
        dataTemp[z-i+1] = '\0';
        j=0;

        title = splitTitle(dataTemp); /*Asking for a split in order to obtain title string*/
        free(dataTemp);

        re = pcre_compile(specialRegex, 0, &error,&erroffset,NULL);
        pcre_exec(re, NULL, references, strlen(references),h3EndIndex, 0, ovector, OVECCOUNT);
        specialAIndex = ovector[1];

        rc = pcre_exec(pagesRe, NULL, references, strlen(references),h3EndIndex, 0, ovector, OVECCOUNT);
        pagesIndex = ovector[0];

        if(( specialAIndex < pagesIndex) || (checkedPap == counter - 1)) {
        /*There are authors or last reference (case '<br> occurence index < <a href= next occurence index')*/
        sizeString = pagesIndex - h3EndIndex;
        dataTemp = (char*)malloc((sizeString + 3)*sizeof(char));
        memcpy(dataTemp, &references[h3EndIndex], sizeString);

        alList = splitAuthors(dataTemp); /*Asking for a split in order to obtain autgorslist*/
        free(dataTemp);
        }
        pagesIndex = ovector[1] + 1; /*<b>Page\\(s\\):</b>*/

        /*Looking for '<' from last '<b>Page\\(s\\):</b>'*/
        rc = pcre_exec(oabRe, NULL, references, strlen(references),pagesIndex, 0, ovector, OVECCOUNT);
        oabReIndex = ovector[0] - 1;
        sizeString = oabReIndex - pagesIndex;
        dataTemp = (char*)malloc(sizeString+1*sizeof(char));
        memcpy(dataTemp, &references[pagesIndex], sizeString);
        dataTemp[sizeString]='\0';

        pages = splitPP(dataTemp, sizeString);  /*Asking for a split in order to obtain pages string*/

        rc = pcre_exec(pdfRe, NULL, references, strlen(references),oabReIndex, 0, ovector, OVECCOUNT);
        if(rc) {
            /*if there is an electronic paper (matching of "\">PDF</a>" regex)*/
            startUrl = ovector[1] +1;
            pcre_exec(aEndRe, NULL, references, strlen(references),startUrl, 0, ovector, OVECCOUNT);
            endUrl = ovector[0];

            support = (char*) malloc((endUrl - startUrl + 3 + strlen(URLIEEE))*sizeof(char));
            strcpy(support, URLIEEE); /*Here we building dynamically url of eletronic paper*/
            for(i = startUrl, j = strlen(URLIEEE); i < endUrl; i++,j++) {
                support[j] = references[i];
            }
            support[j] = '\0';
        } else {
            support = (char*) malloc((1)*sizeof(char));
            /*By default*/
            support = "";
        }
        insertReferencesInTail(refList, pages, alList, title, support);
        h3Index = oabReIndex; /*Updating of index; next cycle 'll have h3Index update at last match of previous cycle*/
        checkedPap++;

    }
    return 1;
}

/*!
*The function counts number of paper there are in conference page. It does counting looking for '"<!-- CODE FOR CHECKING ABSTR...'
*occurence until regex expression finds a new occurence.
*\param data Datafile of papers part.
*\return Number of paper(s)
*/
int counterPaper(char*data) {
    int i = 0;
    int counter=0;
    int newStartPoint = 0;
    int alreadychecked;
    pcre *re;
    const char *error;
    int erroffset = 0;
    int ovector[OVECCOUNT];
    int rc = 0;
    int beginIndex = 0, endIndex = 0, authorsIndex = 0;
    char *regex = "<!-- CODE FOR CHECKING ABSTRACTPLUS FOR USERS WHO HAVE VIRTUAL JOURNAL PACKAGES -->";

    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (!re) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        exit (-1);
    }

    rc = pcre_exec(re, NULL, data, strlen(data), newStartPoint, 0, ovector, OVECCOUNT);
    newStartPoint = ovector[1];
    alreadychecked = newStartPoint; /*Tag at this index is already counted*/

    /*It needs an increment to avoid 'newstartPoint >= alreadychecked' match a true value (and */
    newStartPoint++;
    while(rc && newStartPoint > alreadychecked) {
        /*There is a new matching of tag '<!-- CODE FOR CHECKING ABSTR...'. Second condition of while
        it's necessary to avoid restart of index at begin of file*/
        if(rc!= -1) {
        counter++;}
        rc = pcre_exec(re, NULL, data, strlen(data),newStartPoint, 0, ovector, OVECCOUNT);
        newStartPoint = ovector[1];
    }
    return counter;
}
/*!
*This function receives a splitted part of big reference file. The function split between
*convenient tags and copyies in a support array the title
*\param data Datafile of title part.
*\return String having title of paper
*/
char* splitTitle(char* data) {
    int i = 0;
    int j = 0;
    int rc = 0;
    int erroffset = 0;
    int startTitle = 0;
    int endTitle = 0;
    pcre *re;
    const char *error;
    int ovector[OVECCOUNT];
    char *regex = "<a.*'>";
    char* supportTitle;


    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    if (!re) {
        fprintf(stderr,"\nPCRE compilation failed at expression offset %d: %s\n", erroffset, error);
        exit (-1);
    }
    rc = pcre_exec(re, NULL, data, strlen(data),0, 0, ovector, OVECCOUNT);
    startTitle = ovector[1];

    regex = "</a>";
    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    rc = pcre_exec(re, NULL, data, strlen(data),startTitle, 0, ovector, OVECCOUNT);
    if(rc) {
        endTitle = ovector[0];
        supportTitle = (char*) malloc((endTitle - startTitle +3)*sizeof(char));

        /*It'll copyies part of string (called "data") between tags '<a.*'>' (startTitle) and '</a>' (endTitle)*/
        for(i = startTitle, j = 0; i < endTitle; i++, j++) {
            supportTitle[j] = data[i];
        }
        supportTitle[j] = '\0';
        return supportTitle;
    }

    else {
        /*Error in matching of the title*/
        exit(-1);
    }

}
/*!
*This function receives a splitted part of big reference file. The function split between
*convenient tags and inserts each author in list. The 'while(1) quits when ther'isnt a
*valid match
*\param data Datafile of authors part.
*\return authorsList, special list having authorsof paper
*/
authorsList splitAuthors(char* data) {

    int i = 0;
    int j =0;
    int startPoint = 0;
    int endpoint = 0;
    pcre *re;
    pcre *brRe;
    pcre *endBr;

    const char *error;
    int erroffset = 0;
    int ovector[OVECCOUNT];
    int rc = 0;
    char *regex = "<b>";
    char *endBregex = "</b>";
    char *support = NULL;
    authorsList al = NULL;

    re = pcre_compile(regex, 0, &error,&erroffset,NULL);
    endBr = pcre_compile(endBregex, 0, &error,&erroffset,NULL);


    if (!re) {
        fprintf(stderr,"\nPCRE 1 compilation failed at expression offset %d: %s\n", erroffset, error);
        exit (-1);
    }

    if (!endBr) {
        fprintf(stderr,"\nPCRE 3 compilation failed at expression offset %d: %s\n", erroffset, error);
        exit (-1);
    }

    while(1) {
        rc = pcre_exec(re, NULL, data, strlen(data), endpoint, 0, ovector, OVECCOUNT);
        startPoint = ovector[1];
        if(rc < 0) {
            /*End of while: Not Matching of "<b>" string*/
            return al;
        }
        rc = pcre_exec(endBr, NULL, data, strlen(data), startPoint, 0, ovector, OVECCOUNT);
        endpoint = ovector[0];
        if(rc < 0) {
            /*End of while: Not Matching of "</b>" string*/
            return al;
        }

        support = (char*) malloc((endpoint - startPoint + 3)*sizeof(char));

        /*It'll copyies part of string (called "data") between tags '<b>' and '</b>'*/
        for(i = startPoint, j = 0; i < endpoint; i++,j++) {
            support[j] = data[i];
        }
        support[j] = '\0';
        insertAuthorsInTail(&al, support);
    }
}
/*!
*This function receives a splitted part of big reference file. The function looks for
special character (like EOF, or blank...) and copyies in a support array pages string.
*\param data Datafile of pages part.
*\param sizeString Length of pages part.
*\return A string having number of pages
*/
char* splitPP(char* data, int sizeString) {
    int i = 0;
    int j = 0;
    int z = 0;
    int startIndex = 0;
    char* supportPP;

    /*These 'for' stop when index-concerning character isn't a blank, or a endline or similar*/
    for(i = startIndex; data[i] == ' ' || data[i] == '\t' || data[i] == '\n' || data[i] == '\r'; i++);
    for(j = sizeString-1; data[j] == ' ' || data[j] == '\t' || data[j] == '\n' || data[j] == '\r'; j--);
    supportPP = (char*) malloc((j - i + 3)*sizeof(char));

    /*At this line, i index and j index are on the pages part */
    for(i, z = 0; i <= j; i++, z++) {
        supportPP[z] = data[i];
    }
    supportPP[z] = '\0';

    return supportPP;
}
