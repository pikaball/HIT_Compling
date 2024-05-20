CC=gcc
LEX=flex
YACC=bison
CFLAGS=-Wall
LDFLAGS=-lfl -ly

TARGET=pcc
MAINFILE=main.c
LEXSOURCE=lexical.l
YACCSOURCE=syntax.y
SEMFILE=semantic.c
CODEFILE=intercode.c

LEXFILE=lex.yy.c
YACCFILE=syntax.tab.c
YACCHEAD=syntax.tab.h



all: clean $(LEXFILE) $(TARGET)

$(TARGET): $(MAINFILE) $(YACCFILE) $(SEMFILE) $(CODEFILE)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(LEXFILE): $(LEXSOURCE)
	$(LEX) $^

$(YACCFILE): $(YACCSOURCE)
	$(YACC) -d $^

.PHONY: clean

- clean:
	- @rm $(TARGET) $(LEXFILE) $(YACCFILE) $(YACCHEAD) *.ir 2>/dev/null

