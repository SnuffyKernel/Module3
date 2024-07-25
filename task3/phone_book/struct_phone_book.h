#ifndef STRUCT_PHONE_BOOK
#define STRUCT_PHONE_BOOK

#define MAX_LEN 25
#define MAX_LEN_LINK 100
#define MAX_LEN_CONTACT 100

struct Phone {
    char phonename[MAX_LEN];
    char phone[MAX_LEN];
};

struct Email {
    char email[MAX_LEN];
};

struct LinkMessage {
    char linkmessage[MAX_LEN_LINK];
};

struct Contact {
    char firstname[MAX_LEN];
    char lastname[MAX_LEN];
    char jobname[MAX_LEN];

    struct Phone* phone[MAX_LEN];
    struct Email* email[MAX_LEN];
    struct LinkMessage* linkmessage[MAX_LEN];

    struct Contact* next;
};

struct Contacts {
    struct Contact* contact[MAX_LEN_CONTACT];
};

#endif //STRUCT_PHONE_BOOK