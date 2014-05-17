CC=g++
CFLAGS=-I.
OBJ = main.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

game-of-life: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
