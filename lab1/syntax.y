%locations
%{
    #include "lex.yy.c"
    TreeNode* root = NULL;
    int Error = 0;
%}

%token LF CR SPACE SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE OCT DEC HEX INT OCTERR DECERR HEXERR INTERR FLOAT FLOATERR ID IDERR

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
Program : ExtDefList                            { root = createNode("Program", SYN);root->lineno = @$.first_line; linkNode(root, $1); }
    ;

ExtDefList : ExtDef ExtDefList                  { $$ = createNode("ExtDefList", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2); }
    |                                           { $$ = createNode("ExtDefList", NULL_SYN);$$->lineno = @$.first_line;  }
    ;

ExtDef : Specifier ExtDecList SEMI              { $$ = createNode("ExtDef", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Specifier SEMI                            { $$ = createNode("ExtDef", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);}
    | Specifier FunDec CompSt                   { $$ = createNode("ExtDef", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);}
    | Specifier error SEMI                      { printf("Discription: error ExtDecList\n");yyerrok;}
    | Specifier ExtDecList error                { printf("Discription: Error ExtDef.\n");}
    | Specifier error                           { printf("Discription: Error ExtDef.\n");}
    ;

ExtDecList : VarDec                             { $$ = createNode("ExtDecList", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    | VarDec COMMA ExtDecList                   { $$ = createNode("ExtDecList", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    ;

Specifier : TYPE                                { $$ = createNode("Specifier", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    | StructSpecifier                           { $$ = createNode("Specifier", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    ;

StructSpecifier : STRUCT OptTag LC DefList RC   { $$ = createNode("StructSpecifier", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);linkNode($$, $4);linkNode($$, $5); }
    | STRUCT Tag                                { $$ = createNode("StructSpecifier", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2); }
    | STRUCT error LC DefList RC                { printf("Discription: Error Struct");yyerrok;}                  
    | STRUCT OptTag LC error RC                 { printf("Discription: Error Struct");yyerrok;}  
    | STRUCT OptTag LC error                    { printf("Discription: Error Struct");yyerrok;}  
    | STRUCT error                              { printf("Discription: Error Struct");yyerrok;}  
    ;

OptTag : ID                                     { $$ = createNode("OptTag", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    |                                           { $$ = createNode("OptTag", NULL_SYN);$$->lineno = @$.first_line;  }
    ;

Tag : ID                                        { $$ = createNode("Tag", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    ;

VarDec : ID                                     { $$ = createNode("VarDec", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    | VarDec LB INT RB                          { $$ = createNode("VarDec", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);linkNode($$, $4); }
    | VarDec LB error RB                        { printf("Discription: Wrong index.\n"); yyerrok;}
    | VarDec LB INT error                       { printf("Discription: Missing ']'\n"); }
    ;

FunDec : ID LP VarList RP                       { $$ = createNode("FunDec", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);linkNode($$, $4); }
    | ID LP RP                                  { $$ = createNode("FunDec", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | ID LP error RP                            { printf("Discription: Wrong args.\n"); yyerrok; }
    ;

VarList : ParamDec COMMA VarList                { $$ = createNode("VarList", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | ParamDec                                  { $$ = createNode("VarList", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    ;

ParamDec : Specifier VarDec                     { $$ = createNode("VarList", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2); }
    ;

CompSt : LC DefList StmtList RC                 { $$ = createNode("CompSt", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);linkNode($$, $4); }
    ;

StmtList : Stmt StmtList                        { $$ = createNode("StmtList", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2); }
    |                                           { $$ = createNode("StmtList", NULL_SYN);$$->lineno = @$.first_line;  }
    ;

Stmt : Exp SEMI                                 { $$ = createNode("Stmt", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2); }
    | CompSt                                    { $$ = createNode("Stmt", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    | RETURN Exp SEMI                           { $$ = createNode("Stmt", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | IF LP Exp RP Stmt                         { $$ = createNode("Stmt", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);linkNode($$, $4);linkNode($$, $5); }
    | IF LP Exp RP error                        { printf("Discription: Wrong if Stmt.\n");yyerrok;}
    | IF LP error RP Stmt                       { printf("Discription: Wrong expression.\n"); yyerrok;}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE   { $$ = createNode("Stmt", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);linkNode($$, $4);linkNode($$, $5);}
    | IF LP Exp RP Stmt ELSE Stmt               { $$ = createNode("Stmt", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);linkNode($$, $4);linkNode($$, $5);linkNode($$, $6);linkNode($$, $7); }
    | IF LP error RP Stmt ELSE Stmt             { printf("Discription: Wrong expression.\n"); yyerrok;}
    | WHILE LP Exp RP Stmt                      { $$ = createNode("Stmt", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);linkNode($$, $4);linkNode($$, $5); }
    | WHILE LP error RP Stmt                      { printf("Discription: Wrong expression.\n"); yyerrok;}
    | Exp error                                 { printf("Discription: Missing ';'\n");}
    | RETURN Exp error                          { printf("Discription: Missing ';'\n");}
    ;

DefList : Def DefList                           { $$ = createNode("DefList", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2); }
    |                                           { $$ = createNode("DefList", NULL_SYN);$$->lineno = @$.first_line;  }
    ;

Def : Specifier DecList SEMI                    { $$ = createNode("Def", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Specifier DecList error                   { printf("Discription: Missing ';'\n"); }
    ;

DecList : Dec                                   { $$ = createNode("DecList", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    | Dec COMMA DecList                         { $$ = createNode("DecList", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    ;

Dec : VarDec                                    { $$ = createNode("Dec", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    | VarDec ASSIGNOP Exp                       { $$ = createNode("Dec", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    ;

Exp : Exp ASSIGNOP Exp                          { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Exp AND Exp                               { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Exp OR Exp                                { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Exp RELOP Exp                             { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Exp PLUS Exp                              { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Exp MINUS Exp                             { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Exp STAR Exp                              { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Exp DIV Exp                               { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | LP Exp RP                                 { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | PLUS Exp                                  { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2); }
    | MINUS Exp                                 { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2); }
    | NOT Exp                                   { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2); }
    | ID LP Args RP                             { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);linkNode($$, $4); }
    | ID LP error RP                            { printf("Discription: Wrong args.\n"); yyerrok; }
    | ID LP RP                                  { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Exp LB Exp RB                             { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3);linkNode($$, $4); }
    | Exp LB error RB                           { printf("Discription: Wrong index.\n"); yyerrok; }
    | Exp DOT ID                                { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | ID                                        { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    | INT                                       { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    | FLOAT                                     { $$ = createNode("Exp", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    ;

Args : Exp COMMA Args                           { $$ = createNode("Args", SYN);$$->lineno = @$.first_line; linkNode($$, $1);linkNode($$, $2);linkNode($$, $3); }
    | Exp                                       { $$ = createNode("Args", SYN);$$->lineno = @$.first_line; linkNode($$, $1); }
    | error COMMA Args                          { printf("Discription: Wrong args.\n"); yyerrok; }
    | Args COMMA error                          { printf("Discription: Wrong args.\n"); }
    ;
%%

void yyerror(const char* msg) { 
    Error = 1;
    printf("Error type B at Line %d: %s\n", yylineno, msg);
}
