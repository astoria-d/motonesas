
BIN=motonesas

OBJS=nesas-main.o tools.o nesas.tab.o nesas.yy.o \
	 6502inst.o

LIBS=-lfl
CFLAGS=-g

all:	$(BIN)

$(BIN): $(OBJS)
	gcc -o $(BIN) $(OBJS) $(LIBS)

.c.o: 
	gcc $(CFLAGS) -c $<

nesas.tab.c: nesas.y
	bison -dv $<

nesas.yy.c: nesas.l
	flex -o nesas.yy.c $<

6502inst.o: 6502inst.c
	./opcode-gen.sh
	gcc $(CFLAGS) -c $<

clean:
	-rm $(OBJS) $(BIN) nesas.yy.c nesas.tab.c nesas.tab.h \
		opcode nesas.output



