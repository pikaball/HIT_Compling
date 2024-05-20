%option yylineno

%{
    #include "tree.h"
    #define YYSTYPE TreeNode*
    #include "syntax.tab.h"
    extern int Error;
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
    int choct(char* text);
    int chhex(char* text);
%}

LF          "\n"
CR          "\r"
SPACE       [ \t]+
SEMI        ";"
COMMA       ","
ASSIGNOP    "="
RELOP       >|<|>=|<=|==|!=
PLUS        "+"
MINUS       "-"
STAR        "*"
DIV         "/"
AND         "&&"
OR          "||"
DOT         "."
NOT         "!"
TYPE        int|float
LP          "("
RP          ")"
LB          "["
RB          "]"
LC          "{"
RC          "}"
STRUCT      struct
RETURN      return
IF          if
ELSE        else
WHILE       while   
OCT         0[1-7][0-7]*
DEC         0|([1-9][0-9]*)
HEX         0[Xx][0-9a-fA-F]+
INT         {OCT}|{DEC}|{HEX}
OCTERR      0([0-7]*[89]+[0-7]*)+
DECERR      00+[0-9]*
HEXERR      0[Xx]([0-9a-fA-F]*[g-zG-Z]+[0-9a-fA-F]*)+
INTERR      {OCTERR}|{DECERR}|{HEXERR}
FLOAT       ([0-9]+\.[0-9]+)|((([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))[Ee][+-]?[0-9]+)
FLOATERR    (((\.[0-9]*)|([0-9]*\.)))|(\.[Ee][+-]?[0-9]+)|((([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))[Ee][+-]?)|((([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))[Ee][+-]?[0-9]*\.[0-9]*)
ID          [_a-zA-Z]+[_0-9a-zA-Z]*
IDERR       [0-9]+[_a-wy-zA-WY-Z]+[_0-9a-zA-Z]*
SINGLECOM   "//"
MULTICOM    "/*"  


%%
{SINGLECOM} { char c = input(); while (c != '\n') c = input(); }
{MULTICOM}  { char c = input(); char d = input(); int rec = 0;
              while (!(c == '*' && d == '/') && c && d)
              { c = d; d = input(); if (c == '/' && d == '*') rec = 1;}
              if (rec || !c || !d) { Error = 1; printf("Error type B at Line %d: Not a legal multiline comment\n", yylineno); } }
{LF}        { yycolumn = 1; }
{CR}        {  }
{SPACE}     {  }
{SEMI}      { yylval = createNode("SEMI", LEX); return SEMI; }
{COMMA}     { yylval = createNode("COMMA", LEX); return COMMA; }
{ASSIGNOP}  { yylval = createNode("ASSIGNOP", LEX); return ASSIGNOP; }
{RELOP}     { yylval = createNode("RELOP", LEX); return RELOP; }
{PLUS}      { yylval = createNode("PLUS", LEX); return PLUS; }
{MINUS}     { yylval = createNode("MINUS", LEX); return MINUS; }
{STAR}      { yylval = createNode("STAR", LEX); return STAR; }
{DIV}       { yylval = createNode("DIV", LEX); return DIV; }
{AND}       { yylval = createNode("AND", LEX); return AND; }
{OR}        { yylval = createNode("OR", LEX); return OR; }
{DOT}       { yylval = createNode("DOT", LEX); return DOT; }
{NOT}       { yylval = createNode("NOT", LEX); return NOT; }
{TYPE}      { yylval = createNode("TYPE", TYPE_LEX); yylval -> strVal = strdup(yytext); return TYPE; }
{LP}        { yylval = createNode("LP", LEX); return LP; }
{RP}        { yylval = createNode("RP", LEX); return RP; }
{LB}        { yylval = createNode("LB", LEX); return LB; }
{RB}        { yylval = createNode("RB", LEX); return RB; }
{LC}        { yylval = createNode("LC", LEX); return LC; }
{RC}        { yylval = createNode("RC", LEX); return RC; }
{STRUCT}    { yylval = createNode("STRUCT", LEX); return STRUCT; }
{RETURN}    { yylval = createNode("RETURN", LEX); return RETURN; }
{IF}        { yylval = createNode("IF", LEX); return IF; }
{ELSE}      { yylval = createNode("ELSE", LEX); return ELSE; }
{WHILE}     { yylval = createNode("WHILE", LEX); return WHILE; }
{OCT}       { yylval = createNode("INT", INT_LEX); yylval -> intVal = choct(yytext); return INT; }
{DEC}       { yylval = createNode("INT", INT_LEX); yylval -> intVal = atoi(yytext); return INT; }
{HEX}       { yylval = createNode("INT", INT_LEX); yylval -> intVal = chhex(yytext); return INT; }
{FLOAT}     { yylval = createNode("FLOAT", FLOAT_LEX); yylval -> floatVal = atof(yytext); return FLOAT; }
{ID}        { yylval = createNode("ID", ID_LEX); yylval -> strVal = strdup(yytext); return ID; }
{INTERR}    { Error = 1; yylval = createNode("INTERR", LEX); printf("Error type A at Line %d: \"%s\" is not a correct int value.\n", yylineno, yytext); return INT;}
{FLOATERR}  { Error = 1; yylval = createNode("FLOATERR", LEX); printf("Error type A at Line %d: \"%s\" is not a correct float value.\n", yylineno, yytext); return FLOAT;}
{IDERR}     { Error = 1; yylval = createNode("IDERR", LEX); printf("Error type A at Line %d: \"%s\" is not a correct id.\n", yylineno, yytext); return ID;}
.           { Error = 1; printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); }
%%

int choct(char* text)
{
    int res = 0;
    int pow = 1;
    int j = strlen(text) - 1;
    while (j)
    {
        res += (text[j] - '0') * pow;
        pow = pow * 8;
        j = j - 1;
    }
    return res;
}

int chhex(char* text)
{
    int res = 0;
    int pow = 1;
    int j = strlen(text) - 1;
    while (j >= 2)
    {
        if (text[j] <= '9') res += (text[j] - 48) * pow;
        else if (text[j] <= 'Z') res += (text[j] - 55) * pow;
        else res += (text[j] - 87) * pow;
        pow = pow * 16;
        j = j - 1;
    }
    return res;
}
