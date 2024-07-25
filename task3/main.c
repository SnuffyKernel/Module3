#include "function_phone_book/function.h"
#include "file/file.h"

void start(struct Contacts* contacts) {
    printf("------------------------\n\t1 - addContact\n\t2 - delContact\n\t3 - listContact\n\t4 - exit\n------------------------\n");

    int c = 0;
    scanf("%d", &c);
    getchar();

    if (c <= 0 || c > 4) {
        error("ICORRECT VALUE");
    } else if (c == 1) {
        addContact(contacts);
    } else if (c == 2) {
        delContact(contacts);
    } else if ( c == 3 ) {
        listContact(contacts);
    } else if ( c == 4 ) {
        exit(0);
    }

}

int main() {
    const char filename[] = "contact.txt";

    struct Contacts contacts;
    initContacts(&contacts);

    loadData(filename, &contacts);

    while (1) {
        start(&contacts);
        saveData(filename, &contacts);
    }
}