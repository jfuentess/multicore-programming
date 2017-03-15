CC=gcc
CFLAGS=-fcilkplus -lcilkrts

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: seq par1 par2 par3

seq: seq.o
	$(CC) -o seq seq.o

par1: par1.o 
	$(CC) -o par1 par1.o $(CFLAGS)

par2: par2.o 
	$(CC) -o par2 par2.o $(CFLAGS)

par3: par3.o 
	$(CC) -o par3 par3.o $(CFLAGS)

clean:
	rm -f *.o

