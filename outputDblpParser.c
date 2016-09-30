#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "outputDblpParser.h"
#include "rep.h"

#define FIRSTURL "outputForDblp/outputForDblp-"
#define SECONDURL ".html"
#define URLLENGTH 40 //count of letters to match the exact pattern "outputForDblp-currentPageNumber.html"

/*!
 *This function is used to produce the output as an HTML page in the DBLP format.
 *
 *\param head
 *              The head of the refernces list to parse
 *\param numberOfPages
 *              The number of pages that have to be parsed (strictly for IEEE contents).
 *              The table of contents of ACM is all in one web page.
 *\param currentPage
 *              The number of the current page that has to be parsed.
 *\param choose
 *              This char can be only 'y' o 'n'. If it's 'n' that mean that all the references
 *              without authors will not be printed in the html output page. If it's 'y' every
 *              reference is printed in the html output page.
 */

void outputDblpParser(referencesList head, int numberOfPages, int currentPage, char choose) {

    FILE *fin;
    int count, countAuthors, countPages;
    int lengthAuthorsList;
    int i;
    char htmlPage[URLLENGTH];
    char pageNumber[3];

    mkdir("./outputForDblp", S_IRWXU|S_IRWXG|S_IRWXO);


    //Building the name of the hmtl page. This depends from the currentPage variable.
    htmlPage[0] = '\0';
    strcat(htmlPage,FIRSTURL);
    sprintf(pageNumber, "%d", currentPage);
    strcat(htmlPage,pageNumber);
    strcat(htmlPage,SECONDURL);

    //printf("%s\n",htmlPage);

    if((fin=fopen(htmlPage,"w"))!=0) {

        fprintf(fin,"%s\n%s\n%s\n%s\n","<html>","<head><link href=\"../style.css\" rel=\"stylesheet\" type=\"text/css\" /></link>","</head>","<body>");

        fprintf(fin,"%s\n","<ul>");

        for(count=1; head; head=head->next,count++) {

            if(!(((head->listAuthors)==NULL)&&choose=='n')){

            fprintf(fin,"%s %d %s\n","<h1>Content number",count,"</h1>");

            fprintf(fin,"%s\n","<li>");

            lengthAuthorsList = authorsListLength(head->listAuthors);

            //INSERT AUTHORS

            //This is the case of a null list and so there are no authors for this paper
            if((head->listAuthors)==NULL){
                fprintf(fin,"%s","No authors:");
            }

            for(countAuthors=0; head->listAuthors; head->listAuthors=(head->listAuthors)->next,countAuthors++) {
                fprintf(fin,"%s",(head->listAuthors)->author);
                if(countAuthors!=(lengthAuthorsList-1)) {
                    fprintf(fin,"%s",", ");
                } else {
                    fprintf(fin,"%s",":");
                }
            }

            //INSERT TITLE OF PAPER
            fprintf(fin,"\n%s %s %s\n","<br/><b>", head->title, "</b><br/>");

            //INSERT RANGE OF PAGES
            //when there are no pages the val of head->pages for IEEE pages will be ""
            if(strcmp((head->pages),"")==0){
                fprintf(fin,"pp. %s","0-");
            }else{
                fprintf(fin,"pp. %s",(head->pages));
            }

            //INSERT URL OF ELETRONIC RESOURCES
            if(strcmp((head->urlEletronicResource),"")!=0) {
                if((head->urlEletronicResource)!=NULL) {
                    fprintf(fin,"%s %s %s %s %s %s\n","<br/>Electronic edition of the paper:<ee>",head->urlEletronicResource,"</ee>", "<a href=\"",head->urlEletronicResource , "\"><img alt=\"Electronic Edition\" src=\"../ee.gif\" border=0 height=\"16\" width=\"16\"></a>");
                }
            }
            fprintf(fin,"\n%s\n","</li>");

          }
        }

        fprintf(fin,"%s\n","</ul>");

        if(numberOfPages!=1) {

             fprintf(fin,"%s\n%s\n","<div id=\"pages\">","<ul>");

            for(i=1; i <=numberOfPages; i++) {
                fprintf(fin,"%s%d%s%d%s\n","<li><a href=\"outputForDblp-",i,".html\">Page ",i,"</a></li>" );
            }
        }

        fprintf(fin,"%s\n%s\n","</ul>","</div>");

        fprintf(fin,"%s\n%s\n","</body>","</hmtl>");

        fclose(fin);
    } else {
        printf("\n%s\n", "Error in writing on the output file");
    }
}
