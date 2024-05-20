#include "semantic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

ListItem* Symbols[TABLE_SIZE];
ListItem* Depchain[MAX_DEPTH];
extern int Error;

unsigned int getHashCode (char* name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
    val = (val << 2) + *name;
    if (i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

void init_symtable()
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbols[i] = NULL;
    }
    for (int i = 0; i < MAX_DEPTH; i++) {
        Depchain[i] = NULL;
    }
    preloadSymbols();
}

void preloadSymbols()
{
    Type* inttype = (Type*)malloc(sizeof(Type));
    inttype->kind = BASIC;
    inttype->basic = 0;
    // int read()
    ListItem* item = (ListItem*)malloc(sizeof(ListItem));
    item->next = NULL;
    item->depnext = NULL;
    item->kind = FUNCTION;
    item->name = "read";
    item->type = inttype;
    item->depth = 0;
    item->param_count = 0;
    item->params = NULL;
    item->isParam = 0;
    insert(item);
    // int write(int w)
    item = (ListItem*)malloc(sizeof(ListItem));
    item->next = NULL;
    item->depnext = NULL;
    item->kind = FUNCTION;
    item->name = "write";
    item->type = inttype;
    item->depth = 0;
    item->param_count = 1;
    FieldList* param = (FieldList*)malloc(sizeof(FieldList));
    param->name = "w";
    param->type = inttype;
    param->next = NULL;
    item->params = param;
    item->isParam = 0;
    insert(item);
}

void test()
{
    ListItem* item = (ListItem*)malloc(sizeof(ListItem));
    item->name = "aaa";
    item->depth = 0;
    insert(item);
    ListItem* res = search("aaa");
    printf("%s %d\n", res->name, res->depth);
    insert(item);
    res = search("aaa");
    printf("%s %d\n", res->name, res->depth);
}

void traversal(TreeNode* node)
{
    //printf("Traversal at line %d, %s\n", node->lineno, node->typeName);
    // find every ExtDef and deal with it 
    if (!node || node->nodeType == NULL_SYN) return;
    if (!strcmp(node->typeName, "ExtDef"))
    {
        handleExtDef(node);
        return;
    }
    TreeNode* tmp = node->firstChild;
    while(tmp)
    {
        traversal(tmp);
        tmp = tmp->next;
    }
}

void handleExtDef(TreeNode* node)
{
    //printf("Traversal extdef at line %d, %s\n", node->lineno, node->typeName);
    /*
    ExtDef → Specifier ExtDecList SEMI  // global vars, insert directively, (goto vardef)
            | Specifier SEMI            // for struct, enter into struct
            | Specifier FunDec CompSt   // enter into func
    */
    TreeNode* branch = node->firstChild->next;
    if (!strcmp(branch->typeName, "ExtDecList")) handleGlobalVar(node);
    else if (!strcmp(branch->typeName, "SEMI")) handleSpecifier(node->firstChild, 0);
    else handleFunction(node);
}

void handleGlobalVar(TreeNode* node)
{
    //printf("Traversal globalvar at line %d, %s\n", node->lineno, node->typeName);
    /*
    ExtDef → Specifier ExtDecList SEMI  // get type(not final), get ids, insert every id, when meet array, rebuild the type [Wait]
    */
    ListItem* item = (ListItem*)malloc(sizeof(ListItem));
    Type* type = handleSpecifier(node->firstChild, 0);
    handleExtDecList(node->firstChild->next, type);
}

Type* handleSpecifier(TreeNode* node, int depth)
{
    
    /*
    Specifier : TYPE    [Done]
    | StructSpecifier   [Wait]
    */
    Type* type = (Type*)malloc(sizeof(Type));
    TreeNode* nodech = node->firstChild;
    if (!strcmp(nodech->typeName, "TYPE"))
    {
        type->kind = BASIC;
        if (!strcmp(nodech->strVal, "int")) type->basic = 0;
        else type->basic = 1;
    }
    else 
    {
        type->kind = STRUCTURE;
        type->structure = handleStructSpecifier(nodech, depth);
        // printf("Traversal specifier at line %d, %s\n", node->lineno, node->typeName);
        // printf("get struct %s\n", type->structure->name);
    }
    return type;
}

StructType* handleStructSpecifier(TreeNode* node, int depth)
{
    /*
    StructSpecifier : STRUCT OptTag LC DefList RC   // build a structtype and insert the tag if tag is not null  [Wait]
    | STRUCT Tag  [Done]  // search and return 
    */
    TreeNode* nodetag = node->firstChild->next;
    if (!strcmp(nodetag->typeName, "OptTag"))
    {
        StructType* strutype = (StructType*)malloc(sizeof(StructType));
        TreeNode* nodedef = nodetag->next->next;
        strutype->structure = handleDefList(nodedef, 1, 0);
        /*
        OptTag : ID
        |
        */
        if (nodetag->nodeType == NULL_SYN) 
            strutype->with_tag = 0;
        else 
        { 
            strutype->with_tag = 1; 
            strutype->name = strdup(nodetag->firstChild->strVal);
            
            ListItem* item = (ListItem*)malloc(sizeof(ListItem));
            item->next = NULL;
            item->depnext = NULL;
            Type* tmptype = (Type*)malloc(sizeof(Type));
            tmptype->kind = STRUCTURE;
            tmptype->structure = strutype;
            item->kind = STRUCTTAG;
            item->name = strdup(nodetag->firstChild->strVal);
            item->type = tmptype;
            item->depth = depth;
            item->isParam = 0;
            ListItem* checker = search(item->name);
            if (checker && checker->depth == depth) { printf("Error type 16 at Line %d: Duplicate tag name\n", node->lineno); Error = 1; }
            else insert(item);
            return strutype;
        }
    }
    else 
    {
        ListItem* res = search(nodetag->firstChild->strVal);
        // assume that struct tags are not same as any varname or funcname
        if (!res) 
        {
            printf("Error type 17 at line %d: Using a struct type without definition\n", node->lineno);
            Error = 1;
            Type* nulltype = (Type*)malloc(sizeof(Type));
            nulltype -> kind = NULL_TYPE;
            return nulltype;
        }
        return res->type->structure;
    }
}

FieldList* handleDefList(TreeNode* node, int in_struct, int depth)
{
    //printf("Traversal deflist at line %d, %s\n", node->lineno, node->typeName);
    /*
    DefList : Def DefList [Wait]
    |   [Done]
    */
    if (node->nodeType == NULL_SYN) return NULL;
    FieldList* nodedef = handleDef(node->firstChild, in_struct, depth);
    FieldList* tmp = nodedef;
    while (tmp->next) tmp = tmp->next;
    tmp->next = handleDefList(node->firstChild->next, in_struct, depth);
    //printfiledlist(nodedef);
    return nodedef;
}

void printfiledlist(FieldList* list)
{
    FieldList* tmp = list;
    int cnt = 0;
    while (tmp)
    {
        printf("Field %d : %s\n", cnt, tmp->name);
        cnt++;
        tmp = tmp->next;
    }
}

FieldList* handleDef(TreeNode* node, int in_struct, int depth)
{
    /*
    int a,b,c;
    Def : Specifier DecList SEMI [Wait]
    */
    Type* type = handleSpecifier(node->firstChild, depth);
    FieldList* declist = handleDecList(node->firstChild->next, type, in_struct, depth);
    // printf("Traversal def at line %d, %s\n", node->lineno, node->typeName);
    // printfiledlist(declist);
    return declist;
}

FieldList* handleDecList(TreeNode* node, Type* type, int in_struct, int depth)
{
    
    /*
    a,b,c
    DecList : Dec [Wait]
    | Dec COMMA DecList [Wait]
    */
    FieldList* declst = handleDec(node->firstChild, type, in_struct, depth);
    if (node->firstChild->next)
        declst->next = handleDecList(node->firstChild->next->next, type, in_struct, depth);
    else
        declst->next = NULL;
    return declst;
}

FieldList* handleDec(TreeNode* node, Type* type, int in_struct, int depth)
{
    // printf("Traversal dec at line %d, %s\n", node->lineno, node->typeName);
    /*
    a
    a=5
    Dec : VarDec  [Done]
    | VarDec ASSIGNOP Exp [Wait]
    if in struct, only build a fieldlist, else , only insert var symbols   
    */
    Type* exptype = NULL;
    if (node->firstChild->next) 
    exptype = handleExp(node->firstChild->next->next, 0);
    FieldList* dec = handleVarDec(node->firstChild, type);
    //printf("vardec-type: %d\n", dec->type->kind);
    if (in_struct)
    {
        if (node->firstChild->next)
            { printf("Error type 15 at line %d: trying to initialize a domian in struct\n", node->lineno); Error = 1; }
        return dec;
    }
    else
    {
        if (exptype && !checkType(dec->type, exptype))
            { printf("Error type 5 at line %d: Type mismatched for assignment\n", node->lineno); Error = 1; }
        ListItem* item = (ListItem*)malloc(sizeof(ListItem));
        item->next = NULL;
        item->depnext = NULL;
        item->kind = VARIABLE;
        item->name = strdup(dec->name);
        item->type = dec->type;
        item->depth = depth;
        item->isParam = 0;
        // printf("prepare to insert %s\n", item->name);
        ListItem* checker = search(item->name);
        // printf("%s\n", item->name);
        //printf("%s %d\n",checker->name, checker->depth);
        if (checker && checker->depth == depth) { printf("Error type 3 at Line %d: Duplicate variable name\n", node->lineno); Error = 1; }
        else insert(item);
        return dec;
    }
}

FieldList* handleVarDec(TreeNode* node, Type* type)
{
    //printf("Traversal vardec at line %d, %s, type=%d\n", node->lineno, node->typeName, type->kind);
    /*
    a
    a[2]
    VarDec : ID  [Done]
    | VarDec LB INT RB   [Done]
    */
    FieldList* vardec = (FieldList*)malloc(sizeof(FieldList));
    if (!strcmp(node->firstChild->typeName, "ID"))
    {
        vardec->name = strdup(node->firstChild->strVal);
        vardec->type = type;
        vardec->next = NULL;
    }
    else
    {
        TreeNode* nodech = node;
        Type* arraytype = type;
        while (strcmp(nodech->firstChild->typeName, "ID"))
        {
            ArrayType* tmparr = (ArrayType*)malloc(sizeof(ArrayType));
            tmparr->size = nodech->firstChild->next->next->intVal;
            tmparr->elem = arraytype;
            Type* tmptype = (Type*)malloc(sizeof(Type));
            tmptype->kind = ARRAY;
            tmptype->array = tmparr;
            arraytype = tmptype;
            nodech = nodech->firstChild;
        }
        vardec->name = strdup(nodech->firstChild->strVal);
        vardec->type = arraytype;
        vardec->next = NULL;
    }
    return vardec;
}

void handleExtDecList(TreeNode* node, Type* type)
{
    //printf("Traversal extdeclist at line %d, %s\n", node->lineno, node->typeName);
    /*
    ExtDecList : VarDec [Done]
    | VarDec COMMA ExtDecList  [Done]
    Global vars
    */
    TreeNode* nodech = node->firstChild;
    FieldList* vardec = handleVarDec(nodech, type);
    ListItem* item = (ListItem*)malloc(sizeof(ListItem));
    item->next = NULL;
    item->depnext = NULL;
    item->kind = VARIABLE;
    item->name = strdup(vardec->name);
    item->type = vardec->type; 
    item->depth = 0;
    item->isParam = 0;
    ListItem* checker = search(item->name);
    if (checker && checker->depth == 0) { printf("Error type 3 at Line %d: Duplicate variable name", node->lineno); Error = 1; }
    else insert(item);
    if (nodech->next) handleExtDecList(nodech->next->next, type);
}

void handleFunction(TreeNode* node)
{
    //printf("Traversal function at line %d, %s\n", node->lineno, node->typeName);
    /*
    ExtDef : Specifier FunDec CompSt

    FunDec : ID LP VarList RP 
    | ID LP RP 
    */
    TreeNode* nodech = node->firstChild;
    Type* functype = handleSpecifier(nodech, 0);
    nodech = nodech->next; // FunDec
    ListItem* item = (ListItem*)malloc(sizeof(ListItem));
    item->next = NULL;
    item->depnext = NULL;
    item->kind = FUNCTION;
    item->name = strdup(nodech->firstChild->strVal);
    item->type = functype;
    item->depth = 0;
    item->isParam = 0;
    TreeNode* branch = nodech->firstChild->next->next;
    FieldList* varlist = NULL; 
    if (!strcmp(branch->typeName, "VarList")) varlist = handleVarList(branch);
    int cnt = 0;
    FieldList* tmp = varlist;
    while (tmp) { cnt++; tmp = tmp->next; }
    item->param_count = cnt;
    item->params = varlist;
    ListItem* checker = search(item->name);
    if (checker && checker->kind==FUNCTION && checker->depth == 0) { printf("Error type 4 at Line %d: Duplicate function definition\n", node->lineno); Error = 1; }
    else insert(item);
    handleCompSt(nodech->next, 1, item->name);
}

FieldList* handleVarList(TreeNode* node)
{
    //printf("Traversal varlist at line %d, %s\n", node->lineno, node->typeName);
    /*
    VarList : ParamDec COMMA VarList [Done]
    | ParamDec [Done]
    */
    FieldList* varlist = handleParamDec(node->firstChild);
    if (node->firstChild->next)
        varlist->next = handleVarList(node->firstChild->next->next);
    return varlist;
}

FieldList* handleParamDec(TreeNode* node)
{
    /*
    ParamDec : Specifier VarDec [Done]
    */
    Type* type = handleSpecifier(node->firstChild, 0);
    FieldList* vardec = handleVarDec(node->firstChild->next, type);
    ListItem* item = (ListItem*)malloc(sizeof(ListItem));
    item->next = NULL;
    item->depnext = NULL;
    item->kind = VARIABLE;
    item->name = strdup(vardec->name);
    item->type = vardec->type;
    item->depth = 1;
    item->isParam = 1;
    ListItem* checker = search(item->name);
    if (checker && checker->depth == 1) { printf("Error type 3 at Line %d: Duplicate variable name\n", node->lineno); Error = 1; }
    else insert(item);
    return vardec;

}

void handleCompSt(TreeNode* node, int depth, char* funcname)
{
    /*
    CompSt : LC DefList StmtList RC
    */
    handleDefList(node->firstChild->next, 0, depth);
    handleStmtList(node->firstChild->next->next, depth, funcname);
    remove_dep(depth);
}

void handleStmtList(TreeNode* node, int depth, char* funcname)
{
    /*
    StmtList : Stmt StmtList  
    |    
    */
    if (node->nodeType == NULL_SYN) return;
    handleStmt(node->firstChild, depth, funcname);
    handleStmtList(node->firstChild->next, depth, funcname);
}

void handleStmt(TreeNode* node, int depth, char* funcname)
{
    //printf("Traversal stmt at line %d, %s, depth=%d\n", node->lineno, node->typeName, depth);
    /*
    Stmt : Exp SEMI 
    | CompSt
    | RETURN Exp SEMI 
    | IF LP Exp RP Stmt
    | IF LP Exp RP Stmt ELSE Stmt
    | WHILE LP Exp RP Stmt
    */
    TreeNode* branch = node->firstChild;
    if (!strcmp(branch->typeName, "CompSt"))
        handleCompSt(branch, depth+1, funcname);
    else if (!strcmp(branch->typeName, "IF") || !strcmp(branch->typeName, "WHILE"))
    {
        Type* type = handleExp(branch->next->next, 0);
        if (type->kind != BASIC || type->basic)
        { printf("Error at line %d: if/while condition error\n", branch->lineno); Error = 1; }
        TreeNode* tmp = branch->next->next->next->next;
        handleStmt(tmp, depth, funcname);
        if (tmp->next) handleStmt(tmp->next->next, depth, funcname);
    }
    else if (!strcmp(branch->typeName, "Exp"))
    {
        handleExp(branch, 0);
    }
    else 
    {
        Type* type = handleExp(branch->next, 0);
        Type* ideal = search(funcname)->type;
        if (!checkType(type, ideal))
        { printf("Error type 8 at line %d: Type mismatched for return\n", node->lineno); Error = 1; }
    }
}

Type* handleExp(TreeNode* node, int left)
{
    //printf("Traversal exp at line %d, %s, left=%d\n", node->lineno, node->typeName, left);
    /*
    Exp → Exp ASSIGNOP Exp  done.
    | Exp AND Exp           done.
    | Exp OR Exp            done.
    | Exp RELOP Exp         done.
    | Exp PLUS Exp          done.
    | Exp MINUS Exp         done.
    | Exp STAR Exp          done.
    | Exp DIV Exp           done.
    | LP Exp RP         done.
    | MINUS Exp         done.
    | NOT Exp           done.
    | ID LP Args RP     done.
    | ID LP RP          done.
    | Exp LB Exp RB     done.
    | Exp DOT ID
    | ID                done.
    | INT               done.
    | FLOAT             done.
    */
    TreeNode* nodech = node->firstChild;
    if (left)
    {
        if (!strcmp(nodech->typeName, "ID"))
        {
            if (nodech->next)
            {
                printf("Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", node->lineno);
                Error = 1;
            }
        }
        else if (!strcmp(nodech->typeName, "Exp"))
        {
            if (strcmp(nodech->next->typeName, "LB") && strcmp(nodech->next->typeName, "DOT"))
            {
                printf("Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", node->lineno);
                Error = 1;
            }
        }
        else
        {
            printf("Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", node->lineno);
            Error = 1;
        }
    }
    if (!strcmp(nodech->typeName, "INT"))
    {
        Type* tmp = (Type*)malloc(sizeof(Type));
        tmp->kind = BASIC;
        tmp->basic = 0;
        return tmp;
    }
    if (!strcmp(nodech->typeName, "FLOAT"))
    {
        Type* tmp = (Type*)malloc(sizeof(Type));
        tmp->kind = BASIC;
        tmp->basic = 1;
        return tmp;
    }
    if (!strcmp(nodech->typeName, "ID"))
    {
        ListItem* res = search(nodech->strVal);
        if (!nodech->next) // single ID
        {
            if (!res || res->kind != VARIABLE) 
            {
                printf("Error type 1 at line %d: undefiened variable\n", node->lineno);
                Error = 1;
                Type* nulltype = (Type*)malloc(sizeof(Type));
                nulltype->kind = NULL_TYPE;
                return nulltype;
            }
            return res->type;
        }
        else // ID([param])
        {
            if (!res) 
            {
                printf("Error type 2 at line %d: undefiened function\n",    node->lineno);
                Error = 1;
                Type* nulltype = (Type*)malloc(sizeof(Type));
                nulltype->kind = NULL_TYPE;
                return nulltype;
            }
            if (res->kind != FUNCTION)
            {
                printf("Error type 11 at line %d: \"%s\" is not a function\n", node->lineno, nodech->strVal);
                Error = 1;
                return res->type;
            }
            //printf("hello\n");
            int cnt = 0;
            FieldList* params = NULL;
            if (nodech->next->next && strcmp(nodech->next->next->typeName, "RP"))
                params = handleArgs(nodech->next->next);
            FieldList* tmp = res->params;
            while (params && tmp)
            {
                if (!checkType(params->type, tmp->type))
                {
                    printf("Error type 9 at line %d: invalid params\n",    node->lineno);
                    Error = 1;
                }
                params = params->next;
                tmp = tmp->next;
                cnt++;
            }
            if (cnt != res->param_count)
            {
                printf("Error type 9 at line %d: invalid params\n",    node->lineno);
                Error = 1;
            }
            return res->type;
        }
    }
    if (!strcmp(nodech->typeName, "MINUS") || !strcmp(nodech->typeName, "NOT"))
    {
        Type* chtype = handleExp(nodech->next, 0);
        if (chtype->kind != BASIC)
            { printf("Error type 7 at line %d: Type mismatched for operands\n", node->lineno); Error = 1; }
        return chtype;
    }
    if (!strcmp(nodech->typeName, "LP"))
        return handleExp(nodech->next, 0);
    TreeNode* branch = nodech->next;
    if (!strcmp(branch->typeName, "ASSIGNOP"))
    {
        Type* a = handleExp(nodech, 1);
        Type* b = handleExp(branch->next, 0);
        if (!checkType(a, b))
        {
            printf("Error type 5 at line %d: Type mismatched for assignment\n", node->lineno);
            Error = 1;
        }
        return b;
    }
    if (!strcmp(branch->typeName, "AND") || !strcmp(branch->typeName, "OR"))
    {
        Type* a = handleExp(nodech, 0);
        Type* b = handleExp(branch->next, 0);
        if (a->kind != BASIC || b->kind != BASIC || a->basic || b->basic)
            { printf("Error type 7 at line %d: Type mismatched for operands\n", node->lineno); Error = 1; }
        Type* tmptype = (Type*)malloc(sizeof(Type));
        tmptype->kind = BASIC;
        tmptype->basic = 0;
        return tmptype;
    }
    if (!strcmp(branch->typeName, "RELOP"))
    {
        Type* a = handleExp(nodech, 0);
        Type* b = handleExp(branch->next, 0);
        if (!checkType(a,b) || a->kind != BASIC || b->kind != BASIC)
            { printf("Error type 7 at line %d: Type mismatched for operands\n", node->lineno); Error = 1; }
        Type* tmptype = (Type*)malloc(sizeof(Type));
        tmptype->kind = BASIC;
        tmptype->basic = 0;
        return tmptype;
    }
    if (!strcmp(branch->typeName, "PLUS") || !strcmp(branch->typeName, "MINUS") || !strcmp(branch->typeName, "STAR") || !strcmp(branch->typeName, "DIV"))
    {
        Type* a = handleExp(nodech, 0);
        Type* b = handleExp(branch->next, 0);
        if (!checkType(a,b) || a->kind != BASIC || b->kind != BASIC)
            { printf("Error type 7 at line %d: Type mismatched for operands\n", node->lineno); Error = 1; }
        return a;
    }
    if (!strcmp(branch->typeName, "LB"))
    {
        Type* outside = handleExp(nodech, 0);
        Type* inside = handleExp(branch->next, 0);
        if (outside->kind != ARRAY)
        {
            printf("Error type 10 at line %d: not an array\n", node->lineno);
            Error = 1;
            return outside;
        }
        if (inside->kind != BASIC || inside->basic)
        {
            printf("Error type 12 at line %d: wrong index\n", node->lineno);
            Error = 1;
        }
        return outside->array->elem;
    }
    if (!strcmp(branch->typeName, "DOT"))
    {
        Type* exptype = handleExp(nodech, 0);
        if (exptype->kind != STRUCTURE)
        {
            printf("Error type 13 at line %d: not a structure\n", node->lineno);
            Error = 1;
            Type* nulltype = (Type*)malloc(sizeof(Type));
            nulltype->kind = NULL_TYPE;
            return nulltype;
        }
        char* domain = branch->next->strVal;
        FieldList* tmp = exptype->structure->structure;
        while (tmp)
        {
            if (!strcmp(domain, tmp->name)) 
            { 
                // printf("domain:%s\n",domain);
                // printf("typekind %d\n",tmp->type->kind);
                // if (tmp->type->kind == STRUCTURE) printf("struct:%s\n",tmp->type->structure->name);
                return tmp->type;
            }
            tmp = tmp->next;
        }
        printf("Error type 14 at line %d: unknown domain \"%s\" in structure\n", node->lineno, domain);
        Error = 1;
        Type* nulltype = (Type*)malloc(sizeof(Type));
        nulltype->kind = NULL_TYPE;
        return nulltype;
    }
}

int checkType(Type* a, Type* b)
{
    if (a->kind != b->kind) return 0;
    if (a->kind == BASIC)
    {
        if (a->basic != b->basic) return 0;
        return 1;
    }
    else if (a->kind == ARRAY)
    {
        int cnt1 = 0, cnt2 = 0;
        Type* tmp1 = a;
        Type* tmp2 = b;
        while (tmp1->kind == ARRAY) { cnt1++; tmp1 = tmp1->array->elem; }
        while (tmp2->kind == ARRAY) { cnt2++; tmp2 = tmp2->array->elem; }
        if (cnt1 != cnt2) return 0;
        return checkType(tmp1, tmp2);
    }
    else if (a->kind == STRUCTURE)
    {
        if (strcmp(a->structure->name, b->structure->name)) return 0;
        return 1;
    }
    else return 0;
}

FieldList* handleArgs(TreeNode* node)
{
    /*
    Args : Exp COMMA Args 
    | Exp
    */
    FieldList* args = (FieldList*)malloc(sizeof(FieldList));
    args->type = handleExp(node->firstChild, 0);
    args->next = NULL;
    if (node->firstChild->next)
        args->next = handleArgs(node->firstChild->next->next);
    return args;
}

ListItem* search(char* name)
{
    // printf("search %s\n", name);
    unsigned int hash = getHashCode(name);
    //printf("hash:%d\n",hash);
    ListItem* res = Symbols[hash];
    while (res && (strcmp(res->name, name) || !strcmp(res->name, name) && res->removed)) res = res->next;
    return res;
}

void insert(ListItem* symbol)
{
    // printf("start\n");
    // printf("Insert symbol %s, kind=%d, type=%d, depth=%d\n", symbol->name, symbol->kind, symbol->type->kind, symbol->depth);
    // if (symbol->type->kind == 1) { printf("Array: size=%d, type=%d\n",symbol->type->array->size, symbol->type->array->elem->kind); }
    symbol->removed = 0;
    unsigned int hash = getHashCode(symbol->name);
    symbol->next = Symbols[hash];
    Symbols[hash] = symbol;
    int depth = symbol->depth;
    if (!Depchain[depth]) Depchain[depth] = symbol;
    else {
        ListItem* tmp = Depchain[depth];
        while (tmp->depnext) tmp = tmp->depnext;
        tmp->depnext = symbol;
    };
}

void remove_dep(int depth)
{
    ListItem* tmp = Depchain[depth];
    while (tmp)
    {
        ListItem* nxt = tmp->depnext;
        remove_hash(tmp->name, depth);
        tmp = nxt;
    }
    Depchain[depth] = NULL;
}

void remove_hash(char* name, int depth)
{
    // printf("Remove symbol %s, depth=%d\n",name,depth);
    unsigned int hash = getHashCode(name);
    ListItem* pre = Symbols[hash];
    // if (!strcmp(pre->name, name) && pre->depth == depth)
    // {
    //     Symbols[hash] = pre->next;
    //     free(pre);
    //     return;
    // }
    // ListItem* mid = pre->next;
    // ListItem* suf = NULL;
    // while (mid)
    // {
    //     suf = mid->next;
    //     if (!strcmp(mid->name, name) && mid->depth == depth)
    //     {
    //         pre->next = suf;
    //         free(mid);
    //         return;
    //     }
    //     pre = mid;
    //     mid = suf;
    // }
    while (pre && (strcmp(pre->name, name) || pre->depth != depth) || (!strcmp(pre->name, name) && pre->depth == depth && pre->removed)) pre = pre->next;
    if (pre) pre->removed = 1;
}