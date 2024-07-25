#ifndef FUNCTION_PHONE_BOOK
#define FUNCTION_PHONE_BOOK

#include <stdio.h>
#include <unistd.h>
#include "../phone_book/phone_book.h"
#include "../error/error.h"

void initContacts(struct Contacts* contacts);
void addContact(struct Contacts* contacts);
void delContact(struct Contacts* contacts);
void listContact(struct Contacts* contacts);

#endif //FUNCTION_PHONE_BOOK