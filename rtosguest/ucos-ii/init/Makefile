INCLUDES = ../include
OBJECTS	= main.o
MAKEFILE=Makefile

all: init.o

init.o : $(MAKEFILE) $(OBJECTS)
	$(LD) -r -o init.o $(OBJECTS)
	sync


main.o: main.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c main.c

clean:
	rm *.o

