
BIN=motonesas

OBJS=nesas-main.o nesas.tab.o nesas.yy.o \
	 6502inst.o 6502directive.o segment.o

#LIBS=-lfl
#noyywrap option...
LIBS=-L../libs -lmotones

INCLUDE=-I../include

CFLAGS=-g $(INCLUDE)

all:	$(BIN)

$(BIN): $(OBJS) ../libs
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

test:
	make
	./test.sh

clean:
	-rm $(OBJS) $(BIN) nesas.yy.c nesas.tab.c nesas.tab.h \
		opcode nesas.output all-inst.asm *.o



