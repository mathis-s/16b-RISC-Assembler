#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

typedef struct Keyword {

    char* name;
    char* tokenType;
    bool strict;

} Keyword;

Keyword keywords[] =
{
    {"nop",     "OPCode_NOP",    false},
    {"mov",     "OPCode_MOV",    false},
    {"add",     "OPCode_ADD",    false},
    {"sub",     "OPCode_SUB",    false},
    {"and",     "OPCode_AND",    false},
    {"or",      "OPCode_OR",    false},
    {"xor",     "OPCode_XOR",    false},
    {"not",     "OPCode_NOT",    false},
    {"shl",     "OPCode_SHL",    false},
    {"shr",     "OPCode_SHR",    false},
    {"mul",     "OPCode_MUL",       false},
    {"mulh",    "OPCode_MULH",       false},
    {"mulq",    "OPCode_MULQ",        false},
    {"invq",    "OPCode_INVQ",       false},
    {"div",     "OPCode_DIV",    false},
    {"per",     "OPCode_PER",    false},
    {"jmp",     "OPCode_JMP",    false},
    {"divs",     "OPCode_DIVS",    false},
};

typedef struct Node 
{
    char* string;
    struct Node* nodes[95]; // '~' - 31
    Keyword* keyword;

} Node;

void Insert (Node* root, char* string, Keyword* keyword)
{
    size_t index = 0;
    while (strcmp(root->string, string) != 0)
    {
        
        assert(string[index]);

        if (root->nodes[(size_t) (string[index] - ' ')] == NULL) // ' ' is the first ascii char used in the keywords.
            root->nodes[(size_t) (string[index] - ' ')] = calloc(1, sizeof(Node));

        Node* newRoot = root->nodes[(size_t) (string[index] - ' ')];

        size_t newStringLen = strlen(root->string) + 2;
        newRoot->string = malloc(newStringLen);
        strcpy(newRoot->string, root->string);
        newRoot->string[newStringLen - 2] = string[index];
        newRoot->string[newStringLen - 1] = 0;

        root = newRoot;

        index++;
    }

    root->keyword = keyword;
}


void PrintParser (Node* root, size_t depth, bool printReturn)
{
    int numChildren = 0;
    // Only print next switch if the node has ANY children.
    for (size_t i = 0; i < sizeof(root->nodes) / sizeof(Node*); i++)
        if(root->nodes[i] != NULL)
        {
            numChildren++;
        }

    printf("\n");
    if(numChildren > 1)
        printf("switch (code[*i + %zu])\n{\n", depth);    

    if (numChildren != 0)
        for (size_t i = 0; i < sizeof(root->nodes) / sizeof(Node*); i++)
        {
            if(root->nodes[i] != NULL)
            {
                if (numChildren > 1)
                    printf("case \'%c\':", (int)i + ' ');
                else
                    printf("if (code[*i + %zu] == \'%c\') {", depth, (int)i + ' ');
            
                PrintParser(root->nodes[i], depth + 1, numChildren > 1);

                if (numChildren == 1)
                {
                    printf("}\n");
                }
            }
        }

    if(numChildren > 1)
        printf("}\n");

    if (root->keyword != NULL)
    {
        if (root->keyword->strict)
            printf("if (IsNonIDChar(code[*i + %i]))\n{", (int)depth);
        
        printf("*i += %i;\nreturn %s;\n", (int)depth, root->keyword->tokenType);
        
        if (root->keyword->strict)
            printf("}\n");
    }

    else if(printReturn)
        printf("return -1;\n");
}



int main ()
{

    Node* root = calloc(1, sizeof(Node));
    root->string = "";

    for (size_t i = 0; i < sizeof(keywords) / sizeof(Keyword); i++)
    {
        Insert(root, keywords[i].name, &keywords[i]);
    }

    PrintParser(root, 0, true);
}