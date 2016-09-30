#include <stdlib.h>
#include "rep.h"

/*!
 *This function receives the head of the list of authors
 *to calculate the dimension of the list.
 *
 *\param head
 *          The head of the list of the authors for a given reference.
 *\return The length of the list.
 */
int authorsListLength(authorsList head) {
    int cont;

    for(cont=0; head; head=head->next,cont++)
        ;
    return cont;
}

/*!
 *This function insert in the list of authors a given author.
 *
 *\param l
 *          The head of the list that has to be filled with the second parameter.
 *\param author
 *          The name and/or family name of the author that has to be inserted in the list in the first argument.
 */
void insertAuthorsInTail(authorsList *l, char* author) {

    authorsList last;
    authorsList paux;

    paux = (authorsList) malloc(sizeof(tauthors));
    paux->author = author;
    paux->next = NULL;

    if (*l == NULL) {
        *l = paux;
    } else {
        last = *l;
        while (last->next != NULL) {
            last = last->next;
        }

        last->next = paux;
    }
}

/*!
 *This function insert in the list of authors a given list of author,
 *the title of the reference, and the optional url of the eletronic resources.
 *
 *\param l
 *          The head of the list that has to be filled with the other parameters.
 *\param authorsList
 *          The list of authors that realized this reference.
 *\param title
 *          The title of the reference.
 *\param urlEletronicResource
 *          The url of the eletronic resource(ee). If ee is not available the url must be "".
 */
void insertReferencesInTail(referencesList *l, char* p, authorsList a, char* title, char* urlEletronicResource) {

    referencesList last;
    referencesList paux;

    paux = (referencesList)malloc(sizeof(treferences));
    paux->urlEletronicResource = urlEletronicResource;
    paux->title = title;
    paux->pages = p;
    paux->listAuthors = a;
    paux->next = NULL;

    if (*l == NULL) {
        *l = paux;
    } else {
        last = *l;
        while (last->next != NULL) {
            last = last->next;
        }

        last->next = paux;
    }

}
