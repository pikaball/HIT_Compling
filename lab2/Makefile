CC=gcc
LEX=flex
YACC=bison
CFLAGS=-Wall
LDFLAGS=-lfl -ly

TARGET=parser
MAINFILE=main.c
LEXSOURCE=lexical.l
YACCSOURCE=syntax.y
SEMFILE=semantic.c

LEXFILE=lex.yy.c
YACCFILE=syntax.tab.c
YACCHEAD=syntax.tab.h



all: clean $(LEXFILE) $(TARGET)

$(TARGET): $(MAINFILE) $(YACCFILE) $(SEMFILE)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(LEXFILE): $(LEXSOURCE)
	$(LEX) $^

$(YACCFILE): $(YACCSOURCE)
	$(YACC) -d $^

.PHONY: clean

- clean:
	- @rm $(TARGET) $(LEXFILE) $(YACCFILE) $(YACCHEAD) 2>/dev/null

