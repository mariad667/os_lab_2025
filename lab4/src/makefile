CC=gcc
CFLAGS=-I.
LDFLAGS=

.PHONY: all clean

all : parallel_min_max process_memory

parallel_min_max : utils.o find_min_max.o parallel_min_max.c utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS) $(LDFLAGS)

process_memory : process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS) $(LDFLAGS)

parallel_sum : parallel_sum.o sum.o utils.o sum.h utils.h
	$(CC) -o parallel_sum parallel_sum.o sum.o utils.o $(CFLAGS) $(LDFLAGS) $(PTHREAD_FLAGS)

parallel_sum.o : parallel_sum.c sum.h utils.h
	$(CC) -o parallel_sum.o -c parallel_sum.c $(CFLAGS) $(PTHREAD_FLAGS)

sum.o : sum.c sum.h
	$(CC) -o sum.o -c sum.c $(CFLAGS)

utils.o : utils.c utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : find_min_max.c utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

clean :
	rm -f utils.o find_min_max.o parallel_min_max process_memory result_*.txt