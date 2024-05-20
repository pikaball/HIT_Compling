typedef enum {
    SYN,
    NULL_SYN,
    LEX,
    ID_LEX,
    TYPE_LEX,
    INT_LEX,
    FLOAT_LEX
} NodeType;

struct _node {
    union {
        int intVal;
        float floatVal;
        char* strVal;
    };
    char* typeName;
    int lineno;
    NodeType nodeType;
    struct _node* firstChild;
    struct _node* next;
};

typedef struct _node TreeNode;

TreeNode* createNode(char* typeName, NodeType nodeType);
void linkNode(TreeNode* parent, TreeNode* child);
