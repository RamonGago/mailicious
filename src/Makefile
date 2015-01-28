CC = gcc
CFLAGS = -Wall
C_DIR = ../bin/

mailicious : mailicious.o parser.o emfs.o utils.o
	$(CC) $(CFLAGS) mailicious.o parser.o emfs.o utils.o -o $(C_DIR)mailicious
	rm *.o

mailicious.o : mailicious.c parser.h emfs.h utils.h env.h
	$(CC) $(CFLAGS) -c mailicious.c

parser.o : parser.c parser.h utils.h emfs.h env.h
	$(CC) $(CFLAGS) -c parser.c

emfs.o : emfs.c emfs.h utils.h env.h
	$(CC) $(CFLAGS) -c emfs.c

utils.o : utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c
