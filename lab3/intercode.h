#include "semantic.h"
#include <stdio.h>

typedef enum { LABELDEF, FUNCT, ASSIGN, PLUS, MINUS, TIMES, DIV, GOTO, IF, RET, DEC, ARG, CALL, PARAM, READ, WRITE } CodeType;
typedef enum { LABEL, FUNC, IMNUM, VAR, TMPVAR, SIZE, RELOP, ADDR, FETCH } OpType;

typedef struct _operand Operand;
typedef struct _incode Incode;
typedef struct _snippet Snippet;
typedef struct _expcode ExpCode;

struct _operand {
    OpType type;
    char* name; // for func and variable
    int num;
};

struct _incode {
    CodeType type;
    Operand* op1;
    Operand* op2;
    Operand* op3;
    Incode* pre;
    Incode* next;
};

struct _snippet {
    Incode* head;
    Incode* tail;
};

struct _expcode {
    Snippet* precode;
    Operand* ret; // store the temp variable for exp return
    Type* retType;
};

ListItem* interSearch(char* name);

Operand* getOp(OpType type, char* name, int num);

char* printOp(Operand* op);

int getSize(Type* type);

int getOffset(Type* type, char* subname);

Snippet* genCode(CodeType type, Operand* op1, Operand* op2, Operand* op3);

Snippet* linkSnippet(Snippet* pre, Snippet* suff);

int isNullSnippet(Snippet* snippet);

void printCode(Snippet* graph, FILE* fp);

Snippet* translate(TreeNode* node);

Snippet* translateExtDef(TreeNode* node);

// expect node to be a ExtDecList
Snippet* translateGlobalVar(TreeNode* node);

Snippet* translateFunction(TreeNode* node);

Snippet* translateCompSt(TreeNode* node);

Snippet* translateDefList(TreeNode* node);

Snippet* translateDef(TreeNode* node);

Snippet* translateStmtList(TreeNode* node);

Snippet* translateStmt(TreeNode* node);

ExpCode* translateExp(TreeNode* node);

Type* getBaseType(Type* arraytype);

Type* getSubType(Type* structtype, char* subname);