#ifndef REP_H
#define REP_H

/*!
 *This struct is build to realize a list of authors that will contain
 *the authors of a reference. Contains only the name and/or family name of the author.
 */
struct authors {
    char* author;

    struct authors *next;
};

/*!
 *This struct is build to realize a list of reference that will contain
 *all the references of a conference. Contains the pages, the list of authors,
 *the title and the optional eletronic resource of a reference.
 */
struct references {

    char* pages;
    struct authors *listAuthors;
    char* title;
    char* urlEletronicResource;

    struct references *next;
};

typedef struct authors tauthors;
typedef tauthors *authorsList;

typedef struct references treferences;
typedef treferences *referencesList;

void insertAuthorsInTail(authorsList *l, char* author);
void insertReferencesInTail(referencesList *l, char* p, authorsList a, char* title, char* urlEletronicResources);

#endif
