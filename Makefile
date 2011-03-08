CC = gcc
CFLAGS = -g -pedantic -Wall 
HDRS = myutils.h parselib.h #ttylib.h
SRCS = main.c myutils.c parselib.c #ttylib.c
OBJS = main.o myutils.o parselib.o #ttylib.o
PRGRM = mush


$(PRGRM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PRGRM)

$(OBJS): $(HDRS) $(SRCS)
	$(CC) $(CFLAGS) -c $(HDRS) $(SRCS)

clean:
	rm -f *.o *#* *~ *.gch

allclean:
	rm -f *.o *#* *~ *.gch $(PRGRM)

test:
	@echo 'add personal tests'

nico:
	@~pn-cs357/demos/mush

nicoTest:
	@~pn-cs357/demos/tryAsgn06

handin:
	handin pn-cs357 asgn6 $(HDRS) $(SRCS) README Makefile

val:
	valgrind ./$(PRGRM)