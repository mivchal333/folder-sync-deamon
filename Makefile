OBJ = main.o function.o

all: sw_demon

sw_demon: $(OBJ)
	gcc $(OBJ) -o sw_demon

$(OBJ): function.h

.PHONY: clean
clean:
	rm -f *.o

