SHELL = /bin/sh

CC = g++
CPPFLAGS = -Wall -static -Wextra -pedantic -Wshadow -funroll-loops -O3 -DNDEBUG
#CPPFLAGS = -Wall -static -Wextra -pedantic -Wshadow -g2

all: decode

decode:
	$(CC) $(CPPFLAGS) -o decode decode.cpp utils.cpp

clean:
	rm -f *.o; rm -f decode;
