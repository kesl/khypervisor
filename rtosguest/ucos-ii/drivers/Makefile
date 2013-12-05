OBJECTS	= serial.o
MAKEFILE= Makefile
INCLUDES=../include

all: drivers.o

drivers.o : $(OBJECTS)
	$(LD) -r -o drivers.o $(OBJECTS)
	sync

serial.o : serial.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c serial.c
	
clean:
	rm *.o

