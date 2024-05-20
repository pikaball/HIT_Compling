#include "tree.h"
#define TABLE_SIZE 0x3fff
#define MAX_DEPTH 0x100

typedef struct _type Type;
typedef struct _fieldList FieldList;
typedef struct _list_item ListItem;
typedef struct _structtype StructType;
typedef struct _arraytype ArrayType;

struct _type {
    enum { BASIC, ARRAY, STRUCTURE, NULL_TYPE } kind; // NULL for error
    union {
        int basic; // 0 for int, 1 for float
        ArrayType* array;
        StructType* structure;
    };
}; 

struct _arraytype {
    Type* elem;
    int size;
};

struct _structtype {
    int with_tag;
    char* name;
    FieldList* structure;
};

struct _fieldList {
    char* name;
    Type* type;
    FieldList* next;
};

struct _list_item {
    ListItem* next;
    ListItem* depnext;
    enum { VARIABLE, FUNCTION, STRUCTTAG } kind;
    char* name;
    Type* type;
    int depth;
    // for functions
    int param_count;
    FieldList* params;
};

unsigned int getHashCode (char* name);

void init_symtable();

void traversal(TreeNode* node);

ListItem* search(char* name);

void insert(ListItem* symbol);

void remove_dep(int depth);

void remove_hash(char* name, int depth);

void handleExtDef(TreeNode* node);

void handleGlobalVar(TreeNode* node);

Type* handleSpecifier(TreeNode* node, int depth);

StructType* handleStructSpecifier(TreeNode* node, int depth);

FieldList* handleDefList(TreeNode* node, int in_struct, int depth);

FieldList* handleDef(TreeNode* node, int in_struct, int depth);

FieldList* handleDecList(TreeNode* node, Type* type, int in_struct, int depth);

FieldList* handleDec(TreeNode* node, Type* type, int in_struct, int depth);

FieldList* handleVarDec(TreeNode* node, Type* type);

void handleExtDecList(TreeNode* node, Type* type);

void handleFunction(TreeNode* node);

FieldList* handleVarList(TreeNode* node);

FieldList* handleParamDec(TreeNode* node);

void handleCompSt(TreeNode* node, int depth, char* funcname);

void handleStmtList(TreeNode* node, int depth, char* funcname);

void handleStmt(TreeNode* node, int depth, char* funcname);

Type* handleExp(TreeNode* node, int left);

FieldList* handleArgs(TreeNode* node);

int checkType(Type* a, Type* b);

void test();