.PHONY: clean, mrproper
CC = gcc
CFLAGS = -g -Wall

all : float-eval.o binTree.o
	$(CC) $(CFLAGS) $^ -o float-eval

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	rm -f *.o core.*

mrproper : clean
	rm -f float-eval
