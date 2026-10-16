#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 10
#define INF 999999

char names[MAX][50];
int graph[MAX][MAX];
int count = 0;

int findLocation(char name[])
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (strcmp(names[i], name) == 0)
            return i;
    }
    return -1;
}

void addLocation(char name[])
{
    strcpy(names[count], name);
    count++;
}

void addRoad(char from[], char to[], int distance)
{
    if (distance < 0)
    {
        printf("Invalid distance for %s - %s, skipped.\n", from, to);
        return;
    }

    int a = findLocation(from);
    int b = findLocation(to);

    graph[a][b] = distance;
    graph[b][a] = distance;
}

void loadNetwork()
{
    int i, j;
    for (i = 0; i < MAX; i++)
        for (j = 0; j < MAX; j++)
            graph[i][j] = INF;

    addLocation("Central Warehouse");
    addLocation("Distribution Center");
    addLocation("District Hospital");
    addLocation("Health Center A");
    addLocation("Health Center B");
    addLocation("Health Center C");

    addRoad("Central Warehouse", "Distribution Center", 8);
    addRoad("Central Warehouse", "District Hospital", 14);
    addRoad("Distribution Center", "Health Center A", 5);
    addRoad("Distribution Center", "Health Center B", 11);
    addRoad("Health Center A", "District Hospital", 7);
    addRoad("Health Center B", "Health Center C", 3);
    addRoad("Health Center C", "District Hospital", 6);
    addRoad("Health Center B", "District Hospital", 9);
    addRoad("Health Center A", "Health Center C", 4);
}

void displayLocations()
{
    int i;
    printf("\nLocations:\n");
    for (i = 0; i < count; i++)
    {
        printf("  %s\n", names[i]);
    }
}

void dijkstra(int start, int end)
{
    int dist[MAX];
    int visited[MAX];
    int prev[MAX];
    int i, j;

    for (i = 0; i < count; i++)
    {
        dist[i] = INF;
        visited[i] = 0;
        prev[i] = -1;
    }
    dist[start] = 0;

    for (i = 0; i < count; i++)
    {
        int u = -1;
        int minDist = INF;

        for (j = 0; j < count; j++)
        {
            if (!visited[j] && dist[j] < minDist)
            {
                minDist = dist[j];
                u = j;
            }
        }

        if (u == -1)
        {
            break;
        }

        visited[u] = 1;

        for (j = 0; j < count; j++)
        {
            if (graph[u][j] != INF && !visited[j])
            {
                if (dist[u] + graph[u][j] < dist[j])
                {
                    dist[j] = dist[u] + graph[u][j];
                    prev[j] = u;
                }
            }
        }
    }

    if (dist[end] == INF)
    {
        printf("No route exists from %s to %s.\n", names[start], names[end]);
        return;
    }

    int path[MAX];
    int pathLen = 0;
    int cur = end;

    while (cur != -1)
    {
        path[pathLen] = cur;
        pathLen++;
        cur = prev[cur];
    }

    printf("\nShortest Route:\n");
    for (i = pathLen - 1; i >= 0; i--)
    {
        printf("%s", names[path[i]]);
        if (i > 0)
        {
            printf(" -> ");
        }
    }
    printf("\n");
    printf("Total Distance: %d km\n", dist[end]);
}

void findRoute()
{
    char from[50], to[50];

    printf("Start location: ");
    fgets(from, sizeof(from), stdin);
    from[strcspn(from, "\n")] = '\0';

    printf("Destination: ");
    fgets(to, sizeof(to), stdin);
    to[strcspn(to, "\n")] = '\0';

    int a = findLocation(from);
    int b = findLocation(to);

    if (a == -1 || b == -1)
    {
        printf("One or both locations do not exist.\n");
        return;
    }

    if (a == b)
    {
        printf("Start and destination are the same location.\n");
        return;
    }

    dijkstra(a, b);
}

int main()
{
    char choice[10];

    loadNetwork();
    printf("Emergency Medical Supply Routing\n");

    while (1)
    {
        printf("\nl-Locations r-Route q-Quit\n");
        printf("Choice: ");
        fgets(choice, sizeof(choice), stdin);

        char cmd = choice[0];

        if (cmd == 'l')
        {
            displayLocations();
        }
        else if (cmd == 'r')
        {
            findRoute();
        }
        else if (cmd == 'q')
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