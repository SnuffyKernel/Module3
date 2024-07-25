#include "function.h"

void input(char attrib[], char* elem_contact) {
    char buf[MAX_LEN];
    int len = snprintf(buf, sizeof(buf), "%s: ", attrib);
    write(STDOUT_FILENO, buf, len);

    ssize_t byte = read(STDIN_FILENO, elem_contact, MAX_LEN - 1);
    if ( byte >= 0) {
        elem_contact[byte] = '\0';
    }
}

unsigned int input_arr(char attrib[]) {
    char buf[MAX_LEN];
    int len = snprintf(buf, sizeof(buf), "%s: ", attrib);
    write(STDOUT_FILENO, buf, len);

    unsigned int num = 0;
    if (scanf("%d", &num) != 1) {
        return num;
    }
    getchar();
    if (num >= MAX_LEN) error("INCORRECT NUMBER");
    return num;
}

void addContact(struct Contacts* contacts) {
    struct Contact* contact = (struct Contact*)malloc(sizeof(struct Contact));

    input("firstName", contact->firstname);
    if (!strcmp(contact->firstname, "\n")) {
        error("REQUIRED VALUE");
    }

    input("lastname", contact->lastname);
    if (!strcmp(contact->lastname, "\n")) {
        error("REQUIRED VALUE");
    }

    input("jobname", contact->jobname);

    unsigned int num = input_arr("number of phone");
    if (num) {       
        for (struct Phone* phone; num != 0;) {
            phone=(struct Phone*)malloc(sizeof(struct Phone));
            input("phonename",phone->phonename);
            input("phone", phone->phone);

            contact->phone[--num]=phone;
        }
    }

    num = input_arr("number of email");
    if (num) {       
        for (struct Email* email; num != 0;) {
            email=(struct Email*)malloc(sizeof(struct Email));
            input("email", email->email);

            contact->email[--num]=email;
        }
    }

    num = input_arr("number of linkmessage");
    if (num) {       
        for (struct LinkMessage* link; num != 0;) {
            link=(struct LinkMessage*)malloc(sizeof(struct LinkMessage));
            input("linkmessage", link->linkmessage);

            contact->linkmessage[--num]=link;
        }
    }

    contacts->contact[keygen(contact->firstname, contact->lastname)] = contact;
}

void delContact(struct Contacts* contacts) {
    char firstname[MAX_LEN];
    char lastname[MAX_LEN];

    input("firstName", firstname);
    input("lastname", lastname);

    unsigned int index = keygen(firstname, lastname);

    if ( contacts->contact[index] != NULL ) {
        free(contacts->contact[index]);
        contacts->contact[index] = NULL;
        printf("\033[0;32mSUCCESS\033[0m\n");
    } 
    else printf("\033[0;31mNOT FOUND\033[0m\n");
}

void output(const char* atrib, const char* src) {
    char buf[256];
    int len = snprintf(buf, sizeof(buf), "%s: %s", atrib, src);
    write(STDOUT_FILENO, buf, len);

}

void listContact(struct Contacts* contacts) {
    for (int i = 0; i < MAX_LEN_CONTACT; i++) {
        if (contacts->contact[i]) {
            write(STDOUT_FILENO, "------------------------\n", 26);

            output("firstName", contacts->contact[i]->firstname);
            output("lastName", contacts->contact[i]->lastname);
            output("jobName", contacts->contact[i]->jobname);

            for (int j = 0; contacts->contact[i]->phone[j] != NULL; j++) {
                char buf[256];
                int len = snprintf(buf, sizeof(buf), "phone num %d:\n\tphonename: %s\tphone: %s", j + 1, contacts->contact[i]->phone[j]->phonename, contacts->contact[i]->phone[j]->phone);
                write(STDOUT_FILENO, buf, len);
            }
            for (int j = 0; contacts->contact[i]->email[j] != NULL; j++) {
                char buf[256];
                int len = snprintf(buf, sizeof(buf),"email num %d:\n\tphone: %s", j + 1, contacts->contact[i]->email[j]->email);
                write(STDOUT_FILENO, buf, len);
            }
            for (int j = 0; contacts->contact[i]->linkmessage[j] != NULL; j++) {
                char buf[256];
                int len = snprintf(buf, sizeof(buf),"linkmessage num %d:\n\tlinkmessage: %s", j + 1, contacts->contact[i]->linkmessage[j]->linkmessage);
                write(STDOUT_FILENO, buf, len);
            }
        }
    }
}