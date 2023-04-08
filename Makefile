CC=gcc
CFLAGS=-std=c11 -Wall -pedantic -g 

proj1: proj1.h proj1.c
	${CC} -o $@ ${CFLAGS} $^


