#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENTS 25

struct Event
{
    int id;
    char type[30];
    char deviceID[30];
    char timestamp[30];
    char severity[20];
    struct Event *next;
    struct Event *prev;
};

struct Event *head = NULL;
struct Event *tail = NULL;
struct Event *current = NULL;
int count = 0;
int nextID = 1;

void addEvent(char type[], char deviceID[], char timestamp[], char severity[])
{
    struct Event *e = malloc(sizeof(struct Event));
    if (e == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    e->id = nextID++;
    strcpy(e->type, type);
    strcpy(e->deviceID, deviceID);
    strcpy(e->timestamp, timestamp);
    strcpy(e->severity, severity);
    e->next = NULL;
    e->prev = NULL;

    if (head == NULL)
    {
        head = e;
        tail = e;
        current = e;
    }
    else
    {
        tail->next = e;
        e->prev = tail;
        tail = e;
    }

    count++;

    if (count > MAX_EVENTS)
    {
        struct Event *old = head;
        head = head->next;
        if (head != NULL)
            head->prev = NULL;

        if (current == old)
            current = head;

        free(old);
        count--;
    }

    printf("Event %d added.\n", e->id);
}

void showCurrent()
{
    if (current == NULL)
    {
        printf("No events to show.\n");
        return;
    }

    printf("\nEvent ID: %d\n", current->id);
    printf("Type: %s\n", current->type);
    printf("Device: %s\n", current->deviceID);
    printf("Timestamp: %s\n", current->timestamp);
    printf("Severity: %s\n", current->severity);
}

void nextEvent()
{
    if (current == NULL)
    {
        printf("No events to show.\n");
        return;
    }
    if (current->next == NULL)
    {
        printf("Already at the newest event.\n");
        return;
    }
    current = current->next;
    showCurrent();
}

void prevEvent()
{
    if (current == NULL)
    {
        printf("No events to show.\n");
        return;
    }
    if (current->prev == NULL)
    {
        printf("Already at the oldest event.\n");
        return;
    }
    current = current->prev;
    showCurrent();
}

void clearEvents()
{
    struct Event *e = head;
    while (e != NULL)
    {
        struct Event *temp = e;
        e = e->next;
        free(temp);
    }
    head = NULL;
    tail = NULL;
    current = NULL;
    count = 0;
    printf("All events cleared.\n");
}

int main()
{
    char choice[10];
    char type[30], deviceID[30], timestamp[30], severity[20];

    printf("Embedded Device Event Logger\n");

    while (1)
    {
        printf("\nn-Next p-Previous a-Add c-Current x-Clear q-Quit\n");
        printf("Choice: ");
        fgets(choice, sizeof(choice), stdin);

        char cmd = choice[0];

        if (cmd == 'n')
        {
            nextEvent();
        }
        else if (cmd == 'p')
        {
            prevEvent();
        }
        else if (cmd == 'a')
        {
            printf("Event type: ");
            fgets(type, sizeof(type), stdin);
            type[strcspn(type, "\n")] = '\0';

            printf("Device ID: ");
            fgets(deviceID, sizeof(deviceID), stdin);
            deviceID[strcspn(deviceID, "\n")] = '\0';

            printf("Timestamp: ");
            fgets(timestamp, sizeof(timestamp), stdin);
            timestamp[strcspn(timestamp, "\n")] = '\0';

            printf("Severity: ");
            fgets(severity, sizeof(severity), stdin);
            severity[strcspn(severity, "\n")] = '\0';

            addEvent(type, deviceID, timestamp, severity);
        }
        else if (cmd == 'c')
        {
            showCurrent();
        }
        else if (cmd == 'x')
        {
            clearEvents();
        }
        else if (cmd == 'q')
        {
            clearEvents();
            printf("Session saved. Goodbye.\n");
            break;
        }
        else
        {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}