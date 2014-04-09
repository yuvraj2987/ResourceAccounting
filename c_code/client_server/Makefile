#Trees Makefile
#Compiler
CC=gcc
#Options
CFLAGS= -c -Wall -g -std=c99 
LFLAGS= -g -o

all: server client

server: nw_server.o
	$(CC) nw_server.o $(LFLAGS) server

nw_server.o: nw_server.c
	$(CC) $(CFLAGS) nw_server.c

client: nw_client.o
	$(CC) nw_client.o $(LFLAGS) client

nw_client.o: nw_client.c
	$(CC) $(CFLAGS) nw_client.c


clean:
	rm -rf *.o server client *~
