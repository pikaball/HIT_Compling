CC=gcc
LEX=flex
YACC=bison
CFLAGS=-Wall
LDFLAGS=-lfl -ly

TARGET=parser
DBGTARGET=debug
MAINFILE=main.c
LEXSOURCE=lexical.l
YACCSOURCE=syntax.y

LEXFILE=lex.yy.c
YACCFILE=syntax.tab.c
YACCHEAD=syntax.tab.h


all: clean $(LEXFILE) $(TARGET)

$(TARGET): $(MAINFILE) $(YACCFILE)
	sed -i 's|^\s*extern int yydebug;|//extern int yydebug;|' $(MAINFILE); \
	sed -i 's|^\s*yydebug = 1;|//yydebug = 1;|' $(MAINFILE); \
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(LEXFILE): $(LEXSOURCE)
	$(LEX) $^

$(YACCFILE): $(YACCSOURCE)
	$(YACC) -d $^

.PHONY: clean debug dbgclean

- clean:
	- @rm $(TARGET) $(LEXFILE) $(YACCFILE) $(YACCHEAD) 2>/dev/null

- dbgclean:
	- @rm $(DBGTARGET) $(LEXFILE) $(YACCFILE) $(YACCHEAD) 2>/dev/null

debug_bison: dbgclean
	$(YACC) -d -t $(YACCSOURCE)

debug: debug_bison $(LEXFILE) $(MAINFILE) $(YACCFILE)
	sed -i 's|//extern int yydebug;|extern int yydebug;|' $(MAINFILE); \
	sed -i 's|//yydebug = 1;|yydebug = 1;|' $(MAINFILE); \
	$(CC) $(CFLAGS) $(MAINFILE) $(YACCFILE) -o $(DBGTARGET) $(LDFLAGS)