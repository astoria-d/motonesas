
BIN=motonesln

OBJS=nesln-main.o linker.o


LIBS=-L../libs -lmotones

INCLUDE=-I../include

CFLAGS=-g $(INCLUDE)

all:	$(BIN)

$(BIN): $(OBJS) ../libs
	gcc -o $(BIN) $(OBJS) $(LIBS)

.c.o: 
	gcc $(CFLAGS) -c $<

clean:
	-rm $(OBJS) $(BIN) \



