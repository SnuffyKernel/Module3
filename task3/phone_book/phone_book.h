#ifndef PHONE_BOOK
#define PHONE_BOOK

#include <string.h>
#include "struct_phone_book.h"

unsigned int hash(char* key);
unsigned int keygen(char* firstName, char* lastname);
void initContacts(struct Contacts* contacts);

#endif //PHONE_BOOK