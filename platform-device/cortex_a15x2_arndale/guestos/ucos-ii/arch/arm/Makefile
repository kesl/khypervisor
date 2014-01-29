OBJECTS	= entry.o setup.o os_cpu_a.o os_cpu_c.o irq.o timer.o

INCLUDES=../../include

all : arm.o

arm.o: $(MAKEFILE) $(OBJECTS)
	$(LD) -r -o arm.o $(OBJECTS)
	sync

setup.o: $(MAKEFILE) setup.S
	$(CC) $(CFLAGS) -c setup.S

entry.o: $(MAKEFILE) entry.S
	$(CC) $(CFLAGS) -c entry.S

os_cpu_a.o: $(MAKEFILE) os_cpu_a.S
	$(CC) $(CFLAGS) -c os_cpu_a.S

os_cpu_c.o: os_cpu_c.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c os_cpu_c.c

irq.o: irq.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c irq.c

timer.o: timer.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c timer.c	

clean:
	rm *.o

