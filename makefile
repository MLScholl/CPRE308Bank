#compiler to use
CC=gcc

#all exectubles
ALL=project2

all: $(ALL)

#executables
project2: project2.o Bank.o
	$(CC) -pthread -g -o project2 project2.o Bank.o

#object files
project2.o: project2.c
	$(CC) -g -c appserver.c
Bank.o: Bank.c
	$(CC) -g -c Bank.c

#cleanup files
clean:
	rm -f $(ALL) *.o