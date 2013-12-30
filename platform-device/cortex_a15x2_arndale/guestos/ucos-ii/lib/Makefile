INCLUDES = ../include
OBJECTS	=vsprintf.o string.o printf.o
MAKEFILE=Makefile


all: lib.a

#lib.o : $(MAKEFILE) $(OBJECTS)
	$(LD) -r -o lib.o $(OBJECTS)
	sync

lib.a : $(MAKEFILE) $(OBJECTS)
	$(AR) rcs lib.a $(OBJECTS)
	sync


vsprintf.o : vsprintf.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c vsprintf.c

string.o : string.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c string.c

printf.o : printf.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c printf.c

clean:
	rm *.o
	rm *.a


