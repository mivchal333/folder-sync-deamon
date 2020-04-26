OBJ = main.o funkcje.o

all: sw_demon

sw_demon: $(OBJ)
	gcc $(OBJ) -o sw_demon

$(OBJ): funkcje.h

.PHONY: clean
clean:
	rm -f *.o

