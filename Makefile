CC=g++
CFLAGS=-I.
OBJ = quadtree.o game.o main.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

game-of-life: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm *.o game-of-life
