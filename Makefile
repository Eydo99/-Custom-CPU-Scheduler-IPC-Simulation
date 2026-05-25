CC=gcc
CFLAGS=-Wall -g

all: process_generator scheduler process

process_generator: process_generator.c headers.h
	$(CC) $(CFLAGS) process_generator.c -o process_generator -lrt

scheduler: scheduler.c headers.h
	$(CC) $(CFLAGS) scheduler.c -o scheduler -lrt

process: process.c headers.h
	$(CC) $(CFLAGS) process.c -o process

clean:
	rm -f process_generator scheduler process scheduler.log metrics.txt