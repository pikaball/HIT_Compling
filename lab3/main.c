#include "intercode.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int yyrestart(FILE* f);
extern int yyparse();
//extern int yydebug;
extern TreeNode* root;
extern int Error;
extern Snippet* InterCode;
extern int tmpcnt;
extern int labelcnt;

int flag[128];

int main(int argc, char** argv)
{
//yydebug = 1;
    FILE* f;
    if (argc != 2 && argc != 4) 
    {
        printf("Usage: %s filename [-o outputfile]\n", argv[0]);
        return 1;
    }
    if (!(f = fopen(argv[1], "r")))
    {
        printf("Fail to open %s\n",argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    // if (!Error) printTree(root, 0);
    if (Error) return 1;
    init_symtable();
    //test();
    traversal(root); // semantic check and generate symbol table
    if (Error) return 1;
    tmpcnt = 0;
    labelcnt = 0;
    InterCode = translate(root);
    FILE* fp = stdout;
    if (argc == 4) fp = fopen(argv[3], "w+");
    printCode(InterCode, fp);
    return 0;
}

TreeNode* createNode(char* typeName, NodeType nodeType)
{
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    newNode -> typeName = strdup(typeName);
    newNode -> nodeType = nodeType;
    newNode -> firstChild = NULL;
    newNode -> next = NULL;
    return newNode;
}

void linkNode(TreeNode* parent, TreeNode* child)
{
    if (!parent -> firstChild) 
    {
        parent -> firstChild = child;
    }
    else 
    {
        TreeNode* tmp = parent -> firstChild;
        while (tmp -> next) {tmp = tmp -> next;}
        tmp -> next = child;
    }
}

void printTree(TreeNode* now, int depth)
{
    if (now -> nodeType == NULL_SYN) return;
    flag[depth] = 0;
    for (int i = 0; i < depth; i++) {
        if (i == depth - 1) {
            // 检查当前结点是否是其兄弟中的最后一个可打印结点
            TreeNode* checker = now; 
            int check = 0;
            while (checker -> next) 
            {
                checker = checker -> next;
                check |= checker -> nodeType != NULL_SYN;              
            }
            if (check) {
                printf("├─");
            } else {
                printf("└─");
                flag[i] = 1;
            }
        } else {
            if (!flag[i]) printf("│ ");
            else printf("  ");
        }
    }
    printf("%s", now -> typeName);
    switch (now -> nodeType) {
        case SYN: printf(" (%d)\n", now -> lineno);break;
        case ID_LEX: printf(" : %s\n", now -> strVal);break;
        case INT_LEX: printf(" : %d\n", now -> intVal);break;
        case FLOAT_LEX: printf(" : %f\n", now -> floatVal);break;
        default: printf("\n");
    }
    TreeNode* tmp = now -> firstChild;
    while (tmp)
    {
        //printf("goto depth:%d, now:%d, next:%d\n", depth+1, tmp, tmp->next);
        printTree(tmp, depth + 1);
        tmp = tmp -> next;
    }
}