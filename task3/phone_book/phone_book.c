#include "phone_book.h"

unsigned int hash(char* key) {
    unsigned int index = 0;
    while(*key) {
        index = (index << 5) + *key++;
    }

    return index % MAX_LEN_CONTACT;
}

unsigned int keygen(char* firstName, char* lastname) {
    char key[ MAX_LEN * 2 ];

    strncpy(key, firstName, MAX_LEN * 2 - 1);
    strncat(key, lastname, MAX_LEN * 2 - 1);

    return hash(key);
}

void initContacts(struct Contacts* contacts) {
    for (int i = 0; i < MAX_LEN_CONTACT; i++) {
        contacts->contact[i] = NULL;
    }
}
