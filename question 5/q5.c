#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHARS 256

struct Node
{
    unsigned char ch;
    int freq;
    struct Node *left;
    struct Node *right;
};

int freqTable[MAX_CHARS];
char codes[MAX_CHARS][MAX_CHARS];

struct Node *newNode(unsigned char ch, int freq)
{
    struct Node *n = malloc(sizeof(struct Node));
    n->ch = ch;
    n->freq = freq;
    n->left = NULL;
    n->right = NULL;
    return n;
}

/* Builds the Huffman tree from freqTable using a simple array of
 * active nodes, repeatedly picking the two smallest frequencies and
 * combining them. Not the fastest approach (no heap), but it is easy
 * to follow and fine for log files of this size. */
struct Node *buildTree()
{
    struct Node *nodes[MAX_CHARS];
    int n = 0;
    int i;

    for (i = 0; i < MAX_CHARS; i++)
    {
        if (freqTable[i] > 0)
        {
            nodes[n] = newNode((unsigned char)i, freqTable[i]);
            n++;
        }
    }

    if (n == 0)
    {
        return NULL;
    }

    if (n == 1)
    {
        struct Node *only = nodes[0];
        struct Node *parent = newNode(0, only->freq);
        parent->left = only;
        return parent;
    }

    while (n > 1)
    {
        int min1 = 0, min2 = 1;
        if (nodes[min2]->freq < nodes[min1]->freq)
        {
            int temp = min1;
            min1 = min2;
            min2 = temp;
        }

        for (i = 2; i < n; i++)
        {
            if (nodes[i]->freq < nodes[min1]->freq)
            {
                min2 = min1;
                min1 = i;
            }
            else if (nodes[i]->freq < nodes[min2]->freq)
            {
                min2 = i;
            }
        }

        struct Node *parent = newNode(0, nodes[min1]->freq + nodes[min2]->freq);
        parent->left = nodes[min1];
        parent->right = nodes[min2];

        /* Replace min1 with the new parent, remove min2 by swapping
         * the last node into its place. */
        nodes[min1] = parent;
        nodes[min2] = nodes[n - 1];
        n--;
    }

    return nodes[0];
}

void generateCodes(struct Node *node, char path[], int depth)
{
    if (node == NULL)
    {
        return;
    }

    if (node->left == NULL && node->right == NULL)
    {
        path[depth] = '\0';
        if (depth == 0)
        {
            strcpy(codes[node->ch], "0");
        }
        else
        {
            strcpy(codes[node->ch], path);
        }
        return;
    }

    path[depth] = '0';
    generateCodes(node->left, path, depth + 1);

    path[depth] = '1';
    generateCodes(node->right, path, depth + 1);
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

void compress()
{
    FILE *input = fopen("device_log.txt", "rb");
    if (input == NULL)
    {
        printf("Could not open device_log.txt.\n");
        return;
    }

    unsigned char *data = malloc(1000000);
    int size = 0;
    int c;

    memset(freqTable, 0, sizeof(freqTable));

    while ((c = fgetc(input)) != EOF)
    {
        data[size] = (unsigned char)c;
        freqTable[c]++;
        size++;
    }
    fclose(input);

    if (size == 0)
    {
        printf("device_log.txt is empty. Nothing to compress.\n");
        free(data);
        return;
    }

    printf("\nCharacter frequencies:\n");
    int i;
    for (i = 0; i < MAX_CHARS; i++)
    {
        if (freqTable[i] > 0)
        {
            if (i == '\n')
                printf("  '\\n' : %d\n", freqTable[i]);
            else
                printf("  '%c' : %d\n", i, freqTable[i]);
        }
    }

    struct Node *root = buildTree();

    memset(codes, 0, sizeof(codes));
    char path[MAX_CHARS];
    generateCodes(root, path, 0);

    printf("\nHuffman codes:\n");
    for (i = 0; i < MAX_CHARS; i++)
    {
        if (freqTable[i] > 0)
        {
            if (i == '\n')
                printf("  '\\n' : %s\n", codes[i]);
            else
                printf("  '%c' : %s\n", i, codes[i]);
        }
    }

    FILE *output = fopen("device_log.huff", "wb");

    /* Header: number of distinct characters, then each (char, freq)
     * pair, then the original size. This is enough information to
     * rebuild the exact same tree during decompression. */
    int distinct = 0;
    for (i = 0; i < MAX_CHARS; i++)
    {
        if (freqTable[i] > 0) distinct++;
    }

    fwrite(&distinct, sizeof(int), 1, output);
    for (i = 0; i < MAX_CHARS; i++)
    {
        if (freqTable[i] > 0)
        {
            unsigned char ch = (unsigned char)i;
            fwrite(&ch, sizeof(unsigned char), 1, output);
            fwrite(&freqTable[i], sizeof(int), 1, output);
        }
    }
    fwrite(&size, sizeof(int), 1, output);

    /* Pack the encoded bits into bytes, 8 bits at a time. */
    unsigned char byte = 0;
    int bitCount = 0;

    for (i = 0; i < size; i++)
    {
        char *code = codes[data[i]];
        int j;
        for (j = 0; code[j] != '\0'; j++)
        {
            byte = byte << 1;
            if (code[j] == '1')
            {
                byte = byte | 1;
            }
            bitCount++;

            if (bitCount == 8)
            {
                fwrite(&byte, sizeof(unsigned char), 1, output);
                byte = 0;
                bitCount = 0;
            }
        }
    }

    /* Pad and flush the last partial byte, if any. */
    if (bitCount > 0)
    {
        byte = byte << (8 - bitCount);
        fwrite(&byte, sizeof(unsigned char), 1, output);
    }

    fclose(output);

    FILE *check = fopen("device_log.huff", "rb");
    fseek(check, 0, SEEK_END);
    long compressedSize = ftell(check);
    fclose(check);

    double ratio = (1.0 - ((double)compressedSize / size)) * 100.0;

    printf("\nOriginal size:     %d bytes\n", size);
    printf("Compressed size:   %ld bytes\n", compressedSize);
    printf("Compression ratio: %.2f%%\n", ratio);

    freeTree(root);
    free(data);
}

void decompress()
{
    FILE *input = fopen("device_log.huff", "rb");
    if (input == NULL)
    {
        printf("Could not open device_log.huff. Compress a file first.\n");
        return;
    }

    int distinct;
    fread(&distinct, sizeof(int), 1, input);

    memset(freqTable, 0, sizeof(freqTable));

    int i;
    for (i = 0; i < distinct; i++)
    {
        unsigned char ch;
        int freq;
        fread(&ch, sizeof(unsigned char), 1, input);
        fread(&freq, sizeof(int), 1, input);
        freqTable[ch] = freq;
    }

    int originalSize;
    fread(&originalSize, sizeof(int), 1, input);

    struct Node *root = buildTree();

    FILE *output = fopen("device_log_restored.txt", "wb");

    struct Node *current = root;
    int written = 0;
    int byteVal;

    while (written < originalSize && (byteVal = fgetc(input)) != EOF)
    {
        int bit;
        for (bit = 7; bit >= 0 && written < originalSize; bit--)
        {
            int bitValue = (byteVal >> bit) & 1;

            if (bitValue == 0)
            {
                current = current->left;
            }
            else
            {
                current = current->right;
            }

            if (current->left == NULL && current->right == NULL)
            {
                fputc(current->ch, output);
                written++;
                current = root;
            }
        }
    }

    fclose(input);
    fclose(output);
    freeTree(root);

    printf("Decompressed to device_log_restored.txt (%d bytes).\n", written);
}

void verify()
{
    FILE *original = fopen("device_log.txt", "rb");
    FILE *restored = fopen("device_log_restored.txt", "rb");

    if (original == NULL || restored == NULL)
    {
        printf("Both files must exist to verify. Compress and decompress first.\n");
        if (original) fclose(original);
        if (restored) fclose(restored);
        return;
    }

    int same = 1;
    int a, b;

    while (1)
    {
        a = fgetc(original);
        b = fgetc(restored);

        if (a != b)
        {
            same = 0;
            break;
        }
        if (a == EOF)
        {
            break;
        }
    }

    fclose(original);
    fclose(restored);

    if (same)
    {
        printf("Verification: SUCCESS\n");
        printf("The restored file is identical to the original.\n");
    }
    else
    {
        printf("Verification: FAILED\n");
        printf("The restored file differs from the original.\n");
    }
}

int main()
{
    char choice[10];

    printf("Embedded Firmware Log Compression Utility\n");

    while (1)
    {
        printf("\n1-Compress 2-Decompress 3-Verify 4-Exit\n");
        printf("Choice: ");
        fgets(choice, sizeof(choice), stdin);

        char cmd = choice[0];

        if (cmd == '1')
        {
            compress();
        }
        else if (cmd == '2')
        {
            decompress();
        }
        else if (cmd == '3')
        {
            verify();
        }
        else if (cmd == '4')
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