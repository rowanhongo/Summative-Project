#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_COMMANDS 50

struct Node
{
    char command[50];
    struct Node *left;
    struct Node *right;
};

struct Node *root = NULL;
int commandCount = 0;

struct Node *insert(struct Node *node, char command[], int *inserted)
{
    if (node == NULL)
    {
        struct Node *n = malloc(sizeof(struct Node));
        strcpy(n->command, command);
        n->left = NULL;
        n->right = NULL;
        *inserted = 1;
        return n;
    }

    int cmp = strcmp(command, node->command);

    if (cmp == 0)
    {
        *inserted = 0;
        return node;
    }
    else if (cmp < 0)
    {
        node->left = insert(node->left, command, inserted);
    }
    else
    {
        node->right = insert(node->right, command, inserted);
    }

    return node;
}

void addCommand(char command[])
{
    if (commandCount >= MAX_COMMANDS)
    {
        return;
    }

    int inserted = 0;
    root = insert(root, command, &inserted);

    if (inserted)
    {
        commandCount++;
    }
}

struct Node *search(struct Node *node, char command[])
{
    if (node == NULL)
    {
        return NULL;
    }

    int cmp = strcmp(command, node->command);

    if (cmp == 0)
    {
        return node;
    }
    else if (cmp < 0)
    {
        return search(node->left, command);
    }
    else
    {
        return search(node->right, command);
    }
}

void inorder(struct Node *node)
{
    if (node == NULL)
    {
        return;
    }
    inorder(node->left);
    printf("  %s\n", node->command);
    inorder(node->right);
}

void freeTree(struct Node *node)
{
    if (node == NULL)
    {
        return;
    }
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

/* Basic edit distance, used to decide if two commands are "close enough"
 * to suggest instead of rejecting outright. */
int editDistance(char a[], char b[])
{
    int lenA = strlen(a);
    int lenB = strlen(b);

    int dp[60][60];
    int i, j;

    for (i = 0; i <= lenA; i++)
        dp[i][0] = i;
    for (j = 0; j <= lenB; j++)
        dp[0][j] = j;

    for (i = 1; i <= lenA; i++)
    {
        for (j = 1; j <= lenB; j++)
        {
            if (a[i - 1] == b[j - 1])
            {
                dp[i][j] = dp[i - 1][j - 1];
            }
            else
            {
                int del = dp[i - 1][j] + 1;
                int ins = dp[i][j - 1] + 1;
                int sub = dp[i - 1][j - 1] + 1;

                dp[i][j] = del;
                if (ins < dp[i][j]) dp[i][j] = ins;
                if (sub < dp[i][j]) dp[i][j] = sub;
            }
        }
    }

    return dp[lenA][lenB];
}

/* Looks through the whole tree for the closest command within a
 * small edit-distance threshold. Returns NULL if nothing is close. */
struct Node *closest;
int bestDistance;

void findClosest(struct Node *node, char command[])
{
    if (node == NULL)
    {
        return;
    }

    int d = editDistance(command, node->command);
    if (d < bestDistance)
    {
        bestDistance = d;
        closest = node;
    }

    findClosest(node->left, command);
    findClosest(node->right, command);
}

void logUnknown(char command[])
{
    FILE *log = fopen("audit.log", "a");
    if (log == NULL)
    {
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", t);

    fprintf(log, "%s | UNKNOWN | %s\n", timeStr, command);
    fclose(log);
}

void verifyCommand(char command[])
{
    struct Node *found = search(root, command);

    if (found != NULL)
    {
        printf("COMMAND APPROVED\n");
        return;
    }

    closest = NULL;
    bestDistance = 3;
    findClosest(root, command);

    if (closest != NULL)
    {
        printf("Did you mean: %s?\n", closest->command);
    }
    else
    {
        printf("UNKNOWN COMMAND\n");
    }

    logUnknown(command);
}

void loadCommands(char filename[])
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Could not open %s.\n", filename);
        return;
    }

    char line[50];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0)
        {
            continue;
        }
        addCommand(line);
    }

    fclose(file);
    printf("Loaded %d commands.\n", commandCount);
}

int main()
{
    char choice[10];
    char command[50];

    printf("Network Device Command Registry\n");
    loadCommands("commands.txt");

    while (1)
    {
        printf("\nv-Verify p-Print s-Count x-Clear q-Quit\n");
        printf("Choice: ");
        fgets(choice, sizeof(choice), stdin);

        char cmd = choice[0];

        if (cmd == 'v')
        {
            printf("Enter command: ");
            fgets(command, sizeof(command), stdin);
            command[strcspn(command, "\n")] = '\0';
            verifyCommand(command);
        }
        else if (cmd == 'p')
        {
            if (root == NULL)
            {
                printf("No commands stored.\n");
            }
            else
            {
                printf("Approved commands:\n");
                inorder(root);
            }
        }
        else if (cmd == 's')
        {
            printf("Stored commands: %d\n", commandCount);
        }
        else if (cmd == 'x')
        {
            freeTree(root);
            root = NULL;
            commandCount = 0;
            printf("Registry cleared.\n");
        }
        else if (cmd == 'q')
        {
            freeTree(root);
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