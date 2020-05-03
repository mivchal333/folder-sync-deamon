OBJ = main.o sync_functions.o

all: sync_daemon

sync_daemon: $(OBJ)
	gcc $(OBJ) -o sync_daemon

$(OBJ): sync_functions.h

.PHONY: clean
clean:
	rm -f *.o

