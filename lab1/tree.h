#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    SYN,
    NULL_SYN,
    LEX,
    ID_LEX,
    TYPE_LEX,
    INT_LEX,
    FLOAT_LEX
} NodeType;

struct node {
    union {
        int intVal;
        float floatVal;
        char* strVal;
    };
    char* typeName;
    int lineno;
    NodeType nodeType;
    struct node* firstChild;
    struct node* next;
};

typedef struct node TreeNode;

TreeNode* createNode(char* typeName, NodeType nodeType);
void linkNode(TreeNode* parent, TreeNode* child);
