.PHONY: clean, mrproper
CC = gcc

all : float-eval.o
	$(CC) $^ -o float-eval

%.o : %.c
	$(CC) -c $< -o $@

clean :
	rm -f *.o

mrproper : clean
	rm -f float-eval
