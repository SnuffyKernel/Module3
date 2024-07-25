#ifndef _FILE
#define _FILE

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#include "../error/error.h"
#include "../phone_book/phone_book.h"

#define BUFFER_SIZE 1024

void loadStrCat(char* dest, const char* src);
void loadContact(struct Contacts *contacts, char *buffer);
void loadData(const char *filename, struct Contacts *contacts);

void addBuffer(char *buffer, const char *dest);
void addBufferContact(struct Contacts *contacts, char* buffer);
void saveData(const char *filename, struct Contacts *contacts);

#endif //_FILE