#include "intercode.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern ListItem* Symbols[TABLE_SIZE];

Snippet* InterCode;

int tmpcnt, labelcnt;

ListItem* interSearch(char* name)
{
    unsigned int hash = getHashCode(name);
    ListItem* res = Symbols[hash];
    while (res && (strcmp(res->name, name))) res = res->next;
    return res;
}

Operand* getOp(OpType type, char* name, int num)
{
    Operand* op = (Operand*)malloc(sizeof(Operand));
    op->type = type;
    if (name) op->name = strdup(name);
    else op->name = NULL;
    op->num = num;
    switch (type)
    {
    case LABELDEF:
        labelcnt++;
        op->num = labelcnt;
        break;
    case TMPVAR:
        tmpcnt++;
        op->num = tmpcnt;
        break;
    default:
        break;
    }
    return op;
}

char* printOp(Operand* op)
{
    char* opstr = (char*)malloc(32);
    switch (op->type)
    {
    case LABEL:
        strcpy(opstr, "label");
        break;
    case FUNC:
        strcpy(opstr, op->name);
        return opstr;
    case RELOP:
        strcpy(opstr, op->name);
        return opstr;
    case IMNUM:
        strcpy(opstr, "#");
        break;
    case VAR:
        strcpy(opstr, "v");
        strcat(opstr, op->name);
        return opstr;
    case ADDR:
        strcpy(opstr, "&v");
        strcat(opstr, op->name);
        return opstr;
    case TMPVAR:
        strcpy(opstr, "t");
        break;
    case FETCH:
        strcpy(opstr, "*t");
        break;
    default:
        break;
    }
    char* num = (char*)malloc(8);
    sprintf(num, "%d", op->num);
    strcat(opstr, num);
    free(num);
    return opstr;
}

int getSize(Type* type)
{
    int calc = 0;
    switch (type->kind)
    {
    case ARRAY:
        return type->array->size * getSize(type->array->elem);
    case STRUCTURE:
        FieldList* list = type->structure->structure;
        while (list)
        {
            calc = calc + getSize(list->type);
            list = list->next;
        }
        break;
    default:
        return 4;
    }
    return calc;
}

int getOffset(Type* type, char* subname)
{
    int calc = 0;
    FieldList* list = type->structure->structure;
    while (list && strcmp(list->name, subname)) 
    {
        calc = calc + getSize(list->type);
        list = list->next;
    }
    return calc;
}

Snippet* genCode(CodeType type, Operand* op1, Operand* op2, Operand* op3)
{
    Snippet* snippet = (Snippet*)malloc(sizeof(Snippet));
    Incode* code = (Incode*)malloc(sizeof(Incode));
    code->type = type;
    code->op1 = op1;
    code->op2 = op2;
    code->op3 = op3;
    code->pre = NULL;
    code->next = NULL;
    snippet->head = code;
    snippet->tail = code;
}

int isNullSnippet(Snippet* snippet)
{
    if (!snippet->head && !snippet->tail) return 1;
    return 0;
}

Snippet* linkSnippet(Snippet* pre, Snippet* suff)
{
    if (!suff || isNullSnippet(suff)) return pre;
    if (!pre || isNullSnippet(pre)) return suff;
    pre->tail->next = suff->head;
    suff->head->pre = pre->tail;
    pre->tail = suff->tail;
    return pre;
}

void printCode(Snippet* snippet, FILE* fp)
{
    if (!snippet) return;
    Incode* code = snippet->head;
    while (code)
    {
        switch (code->type)
        {
        case LABELDEF:
            fprintf(fp, "LABEL %s :\n", printOp(code->op1));
            break;
        case FUNCT:
            fprintf(fp, "FUNCTION %s :\n", printOp(code->op1));
            break;
        case ASSIGN:
            fprintf(fp, "%s := %s\n", printOp(code->op1), printOp(code->op2));
            break;
        case PLUS:
            fprintf(fp, "%s := %s + %s\n", printOp(code->op1), printOp(code->op2), printOp(code->op3));
            break;
        case MINUS:
            fprintf(fp, "%s := %s - %s\n", printOp(code->op1), printOp(code->op2), printOp(code->op3));
            break;
        case TIMES:
            fprintf(fp, "%s := %s * %s\n", printOp(code->op1), printOp(code->op2), printOp(code->op3));
            break;
        case DIV:
            fprintf(fp, "%s := %s / %s\n", printOp(code->op1), printOp(code->op2), printOp(code->op3));
            break;
        case GOTO:
            fprintf(fp, "GOTO %s\n", printOp(code->op1));
            break;
        case IF:
            fprintf(fp, "IF %s %s #0 GOTO %s\n", printOp(code->op1), printOp(code->op3), printOp(code->op2));
            break;
        case RET:
            fprintf(fp, "RETURN %s\n", printOp(code->op1));
            break;
        case DEC:
            // fprintf(fp, "%d\n", code->op1->type);
            fprintf(fp, "DEC %s %s\n", printOp(code->op1), printOp(code->op2));
            break;
        case ARG:
            fprintf(fp, "ARG %s\n", printOp(code->op1));
            break;
        case CALL:
            fprintf(fp, "%s := CALL %s\n", printOp(code->op1), printOp(code->op2));
            break;
        case PARAM:
            fprintf(fp, "PARAM %s\n", printOp(code->op1));
            break;
        case READ:
            fprintf(fp, "READ %s\n", printOp(code->op1));
            break;
        case WRITE:
            fprintf(fp, "WRITE %s\n", printOp(code->op1));
            break;
        default:
            break;
        }
        code = code->next;
    }
}

Snippet* translate(TreeNode* node)
{
    Snippet* snippet = (Snippet*)malloc(sizeof(Snippet));
    snippet->head = NULL;
    snippet->tail = NULL;
    if (!node || node->nodeType == NULL_SYN) return NULL;
    if (!strcmp(node->typeName, "ExtDef"))
    {
        snippet = translateExtDef(node);
    }
    TreeNode* tmp = node->firstChild;
    while(tmp)
    {
        snippet = linkSnippet(snippet, translate(tmp));
        tmp = tmp->next;
    }
    return snippet;
}

Snippet* translateExtDef(TreeNode* node)
{
    // printf("translating ExtDef at line %d, node=%s\n",node->lineno, node->typeName);
    Snippet* snippet = (Snippet*)malloc(sizeof(Snippet));
    snippet->head = NULL;
    snippet->tail = NULL;
    TreeNode* branch = node->firstChild->next;
    if (!strcmp(branch->typeName, "ExtDecList")) snippet = linkSnippet(snippet, translateGlobalVar(branch));
    else if (!strcmp(branch->typeName, "FunDec")) snippet = linkSnippet(snippet, translateFunction(node));
    return snippet;
}

Snippet* translateGlobalVar(TreeNode* node)
{
    // printf("translating GlobalVar at line %d, node=%s\n",node->lineno, node->typeName);
    TreeNode* vardec = node->firstChild;
    Snippet* snippet = NULL;
    while (vardec)
    {
        // get ID
        TreeNode* name = vardec->firstChild;
        while (strcmp(name->typeName, "ID")) name = name->firstChild;
        ListItem* symbol = interSearch(name->strVal);
        // printf("get symbol: %s\n", symbol->name);
        if (symbol->type->kind == ARRAY || symbol->type->kind == STRUCTURE) 
            snippet = linkSnippet(snippet, genCode(DEC, getOp(VAR, symbol->name, 0), getOp(SIZE, NULL, getSize(symbol->type)), NULL));
        // find next vardec
        if (vardec->next) vardec = vardec->next->next->firstChild;
        else break;
    }
    return snippet;
}

Snippet* translateFunction(TreeNode* node)
{
    TreeNode* fundec = node->firstChild->next;
    char* funcname = fundec->firstChild->strVal;
    // printf("Start function %s\n", funcname);
    ListItem* funcsymbol = interSearch(funcname);
    Snippet* snippet = genCode(FUNCT, getOp(FUNC, funcname, 0), NULL, NULL);
    FieldList* param = funcsymbol->params;
    while (param)
    {
        snippet = linkSnippet(snippet, genCode(PARAM, getOp(VAR, param->name, 0), NULL, NULL));
        param = param->next;
    }
    TreeNode* compst = fundec->next;
    snippet = linkSnippet(snippet, translateCompSt(compst));
    // printf("End function %s\n", funcname);
    return snippet;
}

Snippet* translateCompSt(TreeNode* node)
{
    Snippet* snippet = NULL;
    TreeNode* deflist = node->firstChild->next;
    TreeNode* stmtlist = deflist->next;
    snippet = linkSnippet(snippet, translateDefList(deflist));
    snippet = linkSnippet(snippet, translateStmtList(stmtlist));
    return snippet;
}

Snippet* translateDefList(TreeNode* node)
{
    // printf("translating DefList at line %d, node=%s\n",node->lineno, node->typeName);
    if (node->nodeType == NULL_SYN) return NULL;
    Snippet* snippet = translateDef(node->firstChild);
    snippet = linkSnippet(snippet, translateDefList(node->firstChild->next));
    return snippet;
}

Snippet* translateDef(TreeNode* node)
{
    // printf("translating Def at line %d, node=%s\n",node->lineno, node->typeName);
    Snippet* snippet = NULL;
    TreeNode* declist = node->firstChild->next;
    TreeNode* dec = declist->firstChild;
    while (dec)
    {
        TreeNode* vardec = dec->firstChild;
        // get ID
        TreeNode* name = vardec->firstChild;
        while (strcmp(name->typeName, "ID")) name = name->firstChild;
        ListItem* symbol = interSearch(name->strVal);
        if (symbol->type->kind == ARRAY || symbol->type->kind == STRUCTURE) 
            snippet = linkSnippet(snippet, genCode(DEC, getOp(VAR, symbol->name, 0), getOp(SIZE, NULL, getSize(symbol->type)), NULL));
        // if having pre assign
        if (vardec->next) 
        {
            ExpCode* expcode = translateExp(vardec->next->next);
            snippet = linkSnippet(snippet, expcode->precode);
            snippet = linkSnippet(snippet, genCode(ASSIGN, getOp(VAR, symbol->name, 0), expcode->ret, NULL));
        }
        // find next dec
        if (dec->next) dec = dec->next->next->firstChild;
        else break;
    }
    return snippet;
}

Snippet* translateStmtList(TreeNode* node)
{
    if (node->nodeType == NULL_SYN) return NULL;
    Snippet* snippet = translateStmt(node->firstChild);
    snippet = linkSnippet(snippet, translateStmtList(node->firstChild->next));
    return snippet;
}

Snippet* translateStmt(TreeNode* node)
{
    // printf("translating Stmt at line %d\n",node->lineno);
    Snippet* snippet = NULL;
    TreeNode* branch = node->firstChild;
    if (!strcmp(branch->typeName, "CompSt")) return translateCompSt(branch);
    if (!strcmp(branch->typeName, "Exp"))
    {
        ExpCode* expcode = translateExp(branch);
        return expcode->precode;
    }
    if (!strcmp(branch->typeName, "RETURN"))
    {
        ExpCode* expcode = translateExp(branch->next);
        snippet = linkSnippet(snippet, expcode->precode);
        snippet = linkSnippet(snippet, genCode(RET, expcode->ret, NULL, NULL));
    }
    if (!strcmp(branch->typeName, "IF"))
    {
        ExpCode* expcode = translateExp(branch->next->next);
        snippet = linkSnippet(snippet, expcode->precode);
        Operand* label0 = getOp(LABEL, NULL, 0);
        Operand* label1 = getOp(LABEL, NULL, 0);
        snippet = linkSnippet(snippet, genCode(IF, expcode->ret, label0, getOp(RELOP, "!=", 0)));
        TreeNode* elsenode = branch->next->next->next->next->next;
        if (elsenode) snippet = linkSnippet(snippet, translateStmt(elsenode->next));
        snippet = linkSnippet(snippet, genCode(GOTO, label1, NULL, NULL));
        snippet = linkSnippet(snippet, genCode(LABELDEF, label0, NULL, NULL));
        snippet = linkSnippet(snippet, translateStmt(branch->next->next->next->next));
        snippet = linkSnippet(snippet, genCode(LABELDEF, label1, NULL, NULL));
    }
    if (!strcmp(branch->typeName, "WHILE"))
    {
        Operand* label0 = getOp(LABEL, NULL, 0);
        Operand* label1 = getOp(LABEL, NULL, 0);
        Operand* label2 = getOp(LABEL, NULL, 0);
        snippet = linkSnippet(snippet, genCode(LABELDEF, label0, NULL, NULL));
        ExpCode* expcode = translateExp(branch->next->next);
        snippet = linkSnippet(snippet, expcode->precode);
        snippet = linkSnippet(snippet, genCode(IF, expcode->ret, label1, getOp(RELOP, "!=", 0)));
        snippet = linkSnippet(snippet, genCode(GOTO, label2, NULL, NULL));
        snippet = linkSnippet(snippet, genCode(LABELDEF, label1, NULL, NULL));
        snippet = linkSnippet(snippet, translateStmt(branch->next->next->next->next));
        snippet = linkSnippet(snippet, genCode(GOTO, label0, NULL, NULL));
        snippet = linkSnippet(snippet, genCode(LABELDEF, label2, NULL, NULL));
    }
    return snippet;
}

ExpCode* translateExp(TreeNode* node)
{
    // printf("translating Exp at line %d\n",node->lineno);
    ExpCode* expcode = (ExpCode*)malloc(sizeof(ExpCode));
    expcode->precode = NULL;
    expcode->ret = NULL;
    expcode->retType = NULL;
    TreeNode* first = node->firstChild;
    TreeNode* branch = first->next;
    if (!strcmp(first->typeName, "INT")) expcode->ret = getOp(IMNUM, NULL, first->intVal);
    else if (!strcmp(first->typeName, "FLOAT")) expcode->ret = getOp(IMNUM, NULL, (int)first->floatVal);
    else if (!strcmp(first->typeName, "ID") && !branch) 
    {
        /*
        if struct or array:
          if not a func param : return &v
          else: return v
        */
        // printf("Now in translate exp on single id , line:%d\n",node->lineno);
        char* varname = first->strVal;
        ListItem* symbol = interSearch(varname);
        if (symbol->type->kind == BASIC) expcode->ret = getOp(VAR, varname, 0);
        else 
        {
            if (symbol->isParam) expcode->ret = getOp(VAR, varname, 0);
            else expcode->ret = getOp(ADDR, varname, 0);
        }
        expcode->retType = symbol->type;
    }
    else if (!strcmp(first->typeName, "ID") && branch)
    {
        TreeNode* args = branch->next;
        char* funcname = first->strVal;
        ListItem* symbol = interSearch(funcname);
        if (!strcmp(funcname, "read"))
        {
            Operand* result = getOp(TMPVAR, NULL ,0);
            expcode->precode = genCode(READ, result, NULL, NULL);
            expcode->ret = result;
        }
        else if (!strcmp(funcname, "write"))
        {
            TreeNode* writearg = args->firstChild;
            ExpCode* argcode = translateExp(writearg);
            expcode->precode = linkSnippet(expcode->precode, argcode->precode);
            expcode->precode = linkSnippet(expcode->precode, genCode(WRITE, argcode->ret, NULL, NULL));
            expcode->ret = getOp(IMNUM, NULL, 0);
        }
        else if (!strcmp(args->typeName, "RP"))
        {
            Operand* funcret = getOp(TMPVAR, NULL, 0);
            expcode->precode = genCode(CALL, funcret, getOp(FUNC, funcname, 0), NULL);
            expcode->ret = funcret;
        }
        else
        {
            TreeNode* exparg = args->firstChild;
            Snippet* precode = NULL;
            Operand* argop[32];
            for (int i = 0; i < symbol->param_count; i++)
            {
                ExpCode* argcode = translateExp(exparg);
                precode = linkSnippet(precode, argcode->precode);
                argop[i] = argcode->ret;
                if (exparg->next) exparg = exparg->next->next->firstChild;
            }
            for (int i = symbol->param_count - 1; i >= 0; i--)
            {
                precode = linkSnippet(precode, genCode(ARG, argop[i], NULL, NULL));
            }
            Operand* funcret = getOp(TMPVAR, NULL, 0);
            precode = linkSnippet(precode, genCode(CALL, funcret, getOp(FUNC, funcname, 0), NULL));
            expcode->precode = precode;
            expcode->ret = funcret;
        }
    }
    else if (!strcmp(first->typeName, "LP"))
    {
        expcode = translateExp(branch);
    }
    else if (!strcmp(first->typeName, "MINUS"))
    {
        Operand* result = getOp(TMPVAR, NULL ,0);
        ExpCode* preexp = translateExp(branch);
        expcode->precode = linkSnippet(preexp->precode, genCode(MINUS, result, getOp(IMNUM, NULL, 0), preexp->ret));
        expcode->ret = result;
        expcode->retType = preexp->retType;
    }
    else if (!strcmp(branch->typeName, "ASSIGNOP"))
    {
        ExpCode* tmp1 = translateExp(first);
        ExpCode* tmp2 = translateExp(branch->next);
        expcode->precode = linkSnippet(tmp1->precode, tmp2->precode);
        expcode->precode = linkSnippet(expcode->precode, genCode(ASSIGN, tmp1->ret, tmp2->ret, NULL));
        expcode->ret = tmp1->ret;
        expcode->retType = tmp1->retType;
    }
    else if (!strcmp(branch->typeName, "PLUS"))
    {
        ExpCode* tmp1 = translateExp(first);
        ExpCode* tmp2 = translateExp(branch->next);
        expcode->precode = linkSnippet(tmp1->precode, tmp2->precode);
        Operand* result = getOp(TMPVAR, NULL, 0);
        expcode->precode = linkSnippet(expcode->precode, genCode(PLUS, result, tmp1->ret, tmp2->ret));
        expcode->ret = result;
        expcode->retType = tmp1->retType;
    }
    else if (!strcmp(branch->typeName, "MINUS"))
    {
        ExpCode* tmp1 = translateExp(first);
        ExpCode* tmp2 = translateExp(branch->next);
        expcode->precode = linkSnippet(tmp1->precode, tmp2->precode);
        Operand* result = getOp(TMPVAR, NULL, 0);
        expcode->precode = linkSnippet(expcode->precode, genCode(MINUS, result, tmp1->ret, tmp2->ret));
        expcode->ret = result;
        expcode->retType = tmp1->retType;
    }
    else if (!strcmp(branch->typeName, "STAR"))
    {
        ExpCode* tmp1 = translateExp(first);
        ExpCode* tmp2 = translateExp(branch->next);
        expcode->precode = linkSnippet(tmp1->precode, tmp2->precode);
        Operand* result = getOp(TMPVAR, NULL, 0);
        expcode->precode = linkSnippet(expcode->precode, genCode(TIMES, result, tmp1->ret, tmp2->ret));
        expcode->ret = result;
        expcode->retType = tmp1->retType;
    }
    else if (!strcmp(branch->typeName, "DIV"))
    {
        ExpCode* tmp1 = translateExp(first);
        ExpCode* tmp2 = translateExp(branch->next);
        expcode->precode = linkSnippet(tmp1->precode, tmp2->precode);
        Operand* result = getOp(TMPVAR, NULL, 0);
        expcode->precode = linkSnippet(expcode->precode, genCode(DIV, result, tmp1->ret, tmp2->ret));
        expcode->ret = result;
        expcode->retType = tmp1->retType;
    }
    else if (!strcmp(branch->typeName, "AND"))
    {
        Operand* label0 = getOp(LABEL, NULL, 0);
        Operand* label1 = getOp(LABEL, NULL, 0);
        Operand* label2 = getOp(LABEL, NULL, 0);
        Operand* label3 = getOp(LABEL, NULL, 0);
        ExpCode* tmp1 = translateExp(first);
        ExpCode* tmp2 = translateExp(branch->next);
        expcode->precode = linkSnippet(tmp1->precode, tmp2->precode);
        Operand* result = getOp(TMPVAR, NULL, 0);
        expcode->precode = linkSnippet(expcode->precode, genCode(IF, tmp1->ret, label0, getOp(RELOP, "!=", 0)));
        expcode->precode = linkSnippet(expcode->precode, genCode(GOTO, label2, NULL, NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(LABELDEF, label0, NULL ,NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(IF, tmp2->ret, label1, getOp(RELOP, "!=", 0)));
        expcode->precode = linkSnippet(expcode->precode, genCode(GOTO, label2, NULL, NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(LABELDEF, label1, NULL ,NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(ASSIGN, result, getOp(IMNUM, NULL, 1), NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(GOTO, label3, NULL, NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(LABELDEF, label2, NULL ,NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(ASSIGN, result, getOp(IMNUM, NULL, 0), NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(LABELDEF, label3, NULL ,NULL));
        expcode->ret = result;
    }
    else if (!strcmp(branch->typeName, "OR"))
    {
        Operand* label0 = getOp(LABEL, NULL, 0);
        Operand* label1 = getOp(LABEL, NULL, 0);
        ExpCode* tmp1 = translateExp(first);
        ExpCode* tmp2 = translateExp(branch->next);
        expcode->precode = linkSnippet(tmp1->precode, tmp2->precode);
        Operand* result = getOp(TMPVAR, NULL, 0);
        expcode->precode = linkSnippet(expcode->precode, genCode(IF, tmp1->ret, label0, getOp(RELOP, "!=", 0)));
        expcode->precode = linkSnippet(expcode->precode, genCode(IF, tmp2->ret, label0, getOp(RELOP, "!=", 0)));
        expcode->precode = linkSnippet(expcode->precode, genCode(ASSIGN, result, getOp(IMNUM, NULL, 0), NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(GOTO, label1, NULL, NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(LABELDEF, label0, NULL ,NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(ASSIGN, result, getOp(IMNUM, NULL, 1), NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(LABELDEF, label1, NULL ,NULL));
        expcode->ret = result;
    }
    else if (!strcmp(branch->typeName, "RELOP"))
    {
        Operand* label0 = getOp(LABEL, NULL, 0);
        Operand* label1 = getOp(LABEL, NULL, 0);
        ExpCode* tmp1 = translateExp(first);
        ExpCode* tmp2 = translateExp(branch->next);
        // printf("RELOP:%s %s %s\n", printOp(tmp1->ret),branch->strVal, printOp(tmp2->ret));
        expcode->precode = linkSnippet(tmp1->precode, tmp2->precode);
        Operand* minus = getOp(TMPVAR, NULL, 0);
        Operand* result = getOp(TMPVAR, NULL, 0);
        expcode->precode = linkSnippet(expcode->precode, genCode(MINUS, minus, tmp1->ret, tmp2->ret));
        expcode->precode = linkSnippet(expcode->precode, genCode(IF, minus, label0, getOp(RELOP, branch->strVal, 0)));
        expcode->precode = linkSnippet(expcode->precode, genCode(ASSIGN, result, getOp(IMNUM, NULL, 0), NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(GOTO, label1, NULL, NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(LABELDEF, label0, NULL ,NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(ASSIGN, result, getOp(IMNUM, NULL, 1), NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(LABELDEF, label1, NULL ,NULL));
        expcode->ret = result;
    }
    else if (!strcmp(first->typeName, "NOT"))
    {
        Operand* label0 = getOp(LABEL, NULL, 0);
        Operand* label1 = getOp(LABEL, NULL, 0);
        ExpCode* tmp = translateExp(branch);
        Operand* result = getOp(TMPVAR, NULL, 0);
        expcode->precode = linkSnippet(expcode->precode, tmp->precode);
        expcode->precode = linkSnippet(expcode->precode, genCode(IF, tmp->ret, label0, getOp(RELOP, "==", 0)));
        expcode->precode = linkSnippet(expcode->precode, genCode(ASSIGN, result, getOp(IMNUM, NULL, 0), NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(GOTO, label1, NULL, NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(LABELDEF, label0, NULL ,NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(ASSIGN, result, getOp(IMNUM, NULL, 1), NULL));
        expcode->precode = linkSnippet(expcode->precode, genCode(LABELDEF, label1, NULL ,NULL));
        expcode->ret = result;
    }
    else if (!strcmp(branch->typeName, "LB"))
    {
        int dimension = 0;
        ExpCode* indexexp[32];
        indexexp[0] = translateExp(branch->next);
        expcode->precode = linkSnippet(expcode->precode, indexexp[dimension]->precode);
        TreeNode* pre = first;
        while (!strcmp(pre->firstChild->typeName, "Exp"))
        {
            dimension++;
            pre = pre->firstChild;
            indexexp[dimension] = translateExp(pre->next->next);
            expcode->precode = linkSnippet(expcode->precode, indexexp[dimension]->precode);
        }
        char* arrayname = pre->firstChild->strVal;
        ExpCode* arrayexp = translateExp(pre);
        ListItem* array = interSearch(arrayname);
        expcode->retType = getBaseType(array->type);
        Operand* calcop[32];
        int dimsize[32];
        Type* arrtype = array->type;
        for (int i = dimension; i >= 0; i--)
        {
            dimsize[i] = arrtype->array->size;
            // printf("dimsize[%d]=%d\n",i,dimsize[i]);
            arrtype = arrtype->array->elem;
        }
        int subsize = getSize(array->type);
        for (int i = dimension; i >= 0; i--)
        {
            calcop[i] = getOp(TMPVAR, NULL, 0);
            subsize = subsize / dimsize[i];
            expcode->precode = linkSnippet(expcode->precode, genCode(TIMES, calcop[i], indexexp[i]->ret, getOp(IMNUM, NULL, subsize)));
        }
        Operand* sizeop = getOp(TMPVAR, NULL, 0);
        expcode->precode = linkSnippet(expcode->precode, genCode(ASSIGN, sizeop, arrayexp->ret, NULL));
        for (int i = dimension; i >= 0; i--)
            expcode->precode = linkSnippet(expcode->precode, genCode(PLUS, sizeop, sizeop, calcop[i]));
        expcode->ret = getOp(FETCH, NULL, sizeop->num);
    }
    else if (!strcmp(branch->typeName, "DOT"))
    {
        // assume that no nested struct 
        char* subname = branch->next->strVal;
        // printf("hello\n");
        ExpCode* structexp = translateExp(first);
        Operand* offset = getOp(IMNUM, NULL, getOffset(getBaseType(structexp->retType), subname));
        Operand* calc = getOp(TMPVAR, NULL, 0);
        expcode->precode = linkSnippet(genCode(ASSIGN, calc, structexp->ret, NULL), genCode(PLUS, calc, calc, offset));
        expcode->ret = getOp(FETCH, NULL, calc->num);
        expcode->retType = getSubType(getBaseType(structexp->retType), subname);
    }
    return expcode;
}

Type* getBaseType(Type* arraytype)
{
    if (arraytype->kind != ARRAY) return arraytype;
    return getBaseType(arraytype->array->elem);
}

Type* getSubType(Type* structtype, char* subname)
{
    FieldList* list = structtype->structure->structure;
    while (list && strcmp(list->name, subname)) list = list->next;
    if (!list) return NULL;
    return list->type;
}