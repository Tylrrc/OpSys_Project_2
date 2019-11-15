CC=gcc
CFLAGS= -std=c99 -Wall -pthread

RUN: main.c vector.c
	gcc -o RUN main.c vector.c $(CFLAGS)
