INCLUDES = ../include
OBJECTS	= ucos_ii.o
#CFLAGS = -c -Wall-fno-builtin 
#-fno-omit-frame-pointer

all: kernel.a

kernel.a : $(OBJECTS)
	$(AR) rcs kernel.a $(OBJECTS)
	sync


ucos_ii.o: $(INCLUDES)/ucos_ii.h						\
		os_dbg.c os_core.c os_flag.c os_mbox.c			\
		os_mutex.c os_q.c os_sem.c os_task.c			\
		os_time.c os_mem.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c ucos_ii.c

clean:
	rm *.o
	rm *.a

