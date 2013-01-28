
BIN=motonesas

OBJS=test.o tools.o nesas.yy.o
LIBS=-lfl
CFLAGS=-g

all:	$(BIN)

$(BIN): $(OBJS)
	gcc -o $(BIN) $(OBJS) $(LIBS)
	./$(BIN) < sample1-short.asm
	./$(BIN) < sample1.asm

.c.o: 
	gcc $(CFLAGS) -c $<

nesas.yy.c: nesas.l
	flex -o nesas.yy.c $<

clean:
	rm $(OBJS) $(BIN) nesas.yy.c



