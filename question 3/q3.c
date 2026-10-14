#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 50

char names[MAX][50];
int matrix[MAX][MAX];
int count = 0;

int findServer(char name[])
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (strcmp(names[i], name) == 0)
            return i;
    }
    return -1;
}

void addServer(char name[])
{
    if (count >= MAX)
    {
        printf("Server limit reached.\n");
        return;
    }
    if (findServer(name) != -1)
    {
        printf("Server already exists.\n");
        return;
    }
    strcpy(names[count], name);
    count++;
    printf("Server added.\n");
}

void removeServer(char name[])
{
    int idx = findServer(name);
    if (idx == -1)
    {
        printf("Server not found.\n");
        return;
    }

    int i, j;
    for (i = idx; i < count - 1; i++)
    {
        strcpy(names[i], names[i + 1]);
        for (j = 0; j < count; j++)
            matrix[i][j] = matrix[i + 1][j];
    }
    for (i = 0; i < count - 1; i++)
    {
        for (j = idx; j < count - 1; j++)
            matrix[i][j] = matrix[i][j + 1];
    }

    count--;
    printf("Server removed.\n");
}

void addDependency(char from[], char to[])
{
    int a = findServer(from);
    int b = findServer(to);
    if (a == -1 || b == -1)
    {
        printf("One or both servers do not exist.\n");
        return;
    }
    matrix[a][b] = 1;
    printf("Dependency added.\n");
}

void removeDependency(char from[], char to[])
{
    int a = findServer(from);
    int b = findServer(to);
    if (a == -1 || b == -1)
    {
        printf("One or both servers do not exist.\n");
        return;
    }
    matrix[a][b] = 0;
    printf("Dependency removed.\n");
}

void queryServer(char name[])
{
    int idx = findServer(name);
    if (idx == -1)
    {
        printf("Server not found.\n");
        return;
    }

    int i, found;

    printf("\nDirect dependencies of %s:\n", name);
    found = 0;
    for (i = 0; i < count; i++)
    {
        if (matrix[idx][i] == 1)
        {
            printf("  %s\n", names[i]);
            found = 1;
        }
    }
    if (!found) printf("  none\n");

    printf("Services depending on %s:\n", name);
    found = 0;
    for (i = 0; i < count; i++)
    {
        if (matrix[i][idx] == 1)
        {
            printf("  %s\n", names[i]);
            found = 1;
        }
    }
    if (!found) printf("  none\n");
}

void printMatrix()
{
    if (count == 0)
    {
        printf("Graph is empty.\n");
        return;
    }

    int i, j;
    printf("\n%-20s", "");
    for (j = 0; j < count; j++)
        printf("%-6.6s", names[j]);
    printf("\n");

    for (i = 0; i < count; i++)
    {
        printf("%-20.20s", names[i]);
        for (j = 0; j < count; j++)
            printf("%-6d", matrix[i][j]);
        printf("\n");
    }
}

void loadStartData()
{
    addServer("Web Server");
    addServer("API Server");
    addServer("Authentication Server");
    addServer("Database Server");
    addServer("Cache Server");
    addServer("Backup Server");

    addDependency("Web Server", "API Server");
    addDependency("Web Server", "Authentication Server");
    addDependency("API Server", "Database Server");
    addDependency("API Server", "Cache Server");
    addDependency("Authentication Server", "Database Server");
    addDependency("Cache Server", "Database Server");
    addDependency("Database Server", "Backup Server");
}

int main()
{
    char choice[10], name[50], from[50], to[50];

    loadStartData();
    printf("Starting server network loaded.\n");

    while (1)
    {
        printf("\na-Add r-Remove c-Create dependency d-Delete dependency q-Query m-Matrix x-Exit\n");
        printf("Choice: ");
        fgets(choice, sizeof(choice), stdin);

        char cmd = choice[0];

        if (cmd == 'a')
        {
            printf("Server name: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = '\0';
            addServer(name);
        }
        else if (cmd == 'r')
        {
            printf("Server name: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = '\0';
            removeServer(name);
        }
        else if (cmd == 'c')
        {
            printf("From: ");
            fgets(from, sizeof(from), stdin);
            from[strcspn(from, "\n")] = '\0';
            printf("To: ");
            fgets(to, sizeof(to), stdin);
            to[strcspn(to, "\n")] = '\0';
            addDependency(from, to);
        }
        else if (cmd == 'd')
        {
            printf("From: ");
            fgets(from, sizeof(from), stdin);
            from[strcspn(from, "\n")] = '\0';
            printf("To: ");
            fgets(to, sizeof(to), stdin);
            to[strcspn(to, "\n")] = '\0';
            removeDependency(from, to);
        }
        else if (cmd == 'q')
        {
            printf("Server name: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = '\0';
            queryServer(name);
        }
        else if (cmd == 'm')
        {
            printMatrix();
        }
        else if (cmd == 'x')
        {
            printf("Goodbye.\n");
            break;
        }
        else
        {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}