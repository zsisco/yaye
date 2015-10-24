#
# ZSisco
# Makefile
#

CC = gcc 
CFLAGS = -Wall -std=c99 -pedantic-errors

default: yaye

yaye: yaye.o 
	$(CC) $(CFLAGS) -o yaye yaye.o

yaye.o: yaye.c
	$(CC) $(CFLAGS) -c yaye.c

clean: 
	$(RM) yaye yaye.o


