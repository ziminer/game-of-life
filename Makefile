CC=g++
CFLAGS=-I.
OBJ = utils.o gameBoard.o game.o main.o
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

game-of-life: $(OBJ)
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

.PHONY: clean

clean:
	rm *.o game-of-life
