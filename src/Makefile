
BIN=motonesas

OBJS=nesas-main.o tools.o nesas.tab.o nesas.yy.o
LIBS=-lfl
CFLAGS=-g

all:	$(BIN)

$(BIN): $(OBJS)
	gcc -o $(BIN) $(OBJS) $(LIBS)
	./$(BIN) < sample1-short.asm
#	./$(BIN) < sample1.asm

.c.o: 
	gcc $(CFLAGS) -c $<

nesas.tab.c: nesas.y
	bison -dv $<

nesas.yy.c: nesas.l
	flex -o nesas.yy.c $<

clean:
	-rm $(OBJS) $(BIN) nesas.yy.c nesas.tab.c nesas.tab.h



