#include "file.h"

void loadStrCat(char* dest, const char* src) {
    strncat(dest, src, MAX_LEN - strlen(dest) - 1);
    strncat(dest, "\n", MAX_LEN - strlen(dest) - 1);
}

void loadContact(struct Contacts *contacts, char *buffer)
{
    char * token = strtok(buffer, "\n");
    while(token != NULL)
    {
        struct Contact *contact = (struct Contact *)malloc(sizeof(struct Contact));

        loadStrCat( contact->firstname, token);
        token = strtok(NULL, "\n");

        loadStrCat( contact->lastname, token);
        token = strtok(NULL, "\n");

        loadStrCat( contact->jobname, token);
        token = strtok(NULL, "\n");

        unsigned int num = atoi(token);
        token = strtok(NULL, "\n");

        for (struct Phone *phone; num != 0;)
        {
            phone = (struct Phone *)malloc(sizeof(struct Phone));
            loadStrCat(phone->phonename, token);
            token = strtok(NULL, "\n");
            loadStrCat(phone->phone, token);
            token = strtok(NULL, "\n");

            contact->phone[--num] = phone;
        }

        num = atoi(token);
        token = strtok(NULL, "\n");

        for (struct Email *email; num != 0;)
        {
            email = (struct Email *)malloc(sizeof(struct Email));

            loadStrCat(email->email, token);
            token = strtok(NULL, "\n");

            contact->email[--num] = email;
        }

        num = atoi(token);
        token = strtok(NULL, "\n");

        for (struct LinkMessage *link; num != 0;)
        {
            link = (struct LinkMessage *)malloc(sizeof(struct LinkMessage));

            loadStrCat(link->linkmessage, token);
            token = strtok(NULL, "\n");

            contact->linkmessage[--num] = link;
        }

        contacts->contact[keygen(contact->firstname, contact->lastname)] = contact;
    }
}

void loadData(const char *filename, struct Contacts *contacts)
{
    int fd;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytesRead;

    fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        error("file");
        return;
    }

    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0)
    {
        buffer[bytesRead] = '\0';
    }

    if (bytesRead == -1)
    {
        error("file");
        close(fd);
        return;
    }

    loadContact(contacts, buffer);

    close(fd);
}

void addBuffer(char *dest, const char *src)
{
    strncat(dest, src, BUFFER_SIZE - strlen(dest) - 1);
    // strncat(buffer, "\n", BUFFER_SIZE - strlen(buffer) - 1);
}

void addBufferContact(struct Contacts *contacts, char *buffer)
{
    for (int i = 0; i < MAX_LEN_CONTACT; i++)
    {
        if (contacts->contact[i])
        {

            addBuffer(buffer, contacts->contact[i]->firstname);
            addBuffer(buffer, contacts->contact[i]->lastname);
            addBuffer(buffer, contacts->contact[i]->jobname);

            int index = 0;
            for (; contacts->contact[i]->phone[index] != NULL; index++)
                ;

            char indexStr[4];
            snprintf(indexStr, sizeof(indexStr), "%d\n", index);
            addBuffer(buffer, indexStr);

            for (int j = 0; j < index; j++)
            {
                addBuffer(buffer, contacts->contact[i]->phone[j]->phonename);
                addBuffer(buffer, contacts->contact[i]->phone[j]->phone);
            }

            index = 0;
            for (; contacts->contact[i]->email[index] != NULL; index++)
                ;

            snprintf(indexStr, sizeof(indexStr), "%d\n", index);
            addBuffer(buffer, indexStr);

            for (int j = 0; j < index; j++)
            {
                addBuffer(buffer, contacts->contact[i]->email[j]->email);
            }

            index = 0;
            for (; contacts->contact[i]->linkmessage[index] != NULL; index++)
                ;

            snprintf(indexStr, sizeof(indexStr), "%d\n", index);
            addBuffer(buffer, indexStr);

            for (int j = 0; j < index; j++)
            {
                addBuffer(buffer, contacts->contact[i]->linkmessage[j]->linkmessage);
            }
        }
    }
}

void saveData(const char *filename, struct Contacts *contacts)
{
    int fd;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        error("file");
        return;
    }

    addBufferContact(contacts, buffer);

    if (lseek(fd, 0, SEEK_SET) == -1)
    {
        error("file");
        close(fd);
        return;
    }

    if (write(fd, buffer, strlen(buffer)) == -1)
    {
        error("file");
        close(fd);
        return;
    }
    close(fd);
}
