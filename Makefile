CC = gcc
CFLAGS = -g -pedantic -Wall 
HDRS = #myutils.h parselib.h ttylib.h
SRCS = #myutils.c parselib.c ttylib.c main.c
OBJS = #myutils.o parselib.o ttylib.o main.o
PRGRM = mush


$(PRGRM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PRGRM)

$(OBJS): $(HDRS) $(SRCS)
	$(CC) $(CFLAGS) -c $(HDRS) $(SRCS)

clean:
	rm -f *.o *#* *~

allclean:
	rm -f *.o *#* *~ $(PRGRM)

test:
	@echo 'add personal tests'

nico:
	@~pn-cs357/demos/mush

nicoTest:
	@~pn-cs357/demos/tryAsgn06

handin:
	handin pn-cs357 asgn6 $(HDRS) $(SRCS) README Makefile

