CC = g++
CFLAGS = -o
LIBS = -lncurses

all:puyo1

puyo1:puyo1.cpp
	$(CC) $(CFLAGS) $@ $^ $(LIBS)

clean:
	rm -f puyo1
	rm -f *.o
