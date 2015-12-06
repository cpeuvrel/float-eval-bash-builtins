.PHONY: clean, mrproper
CC = gcc
CFLAGS = -g -W -Wall -std=c11 -lmpfr -lgmp
CFLAGS_SO = -shared -Wl,-soname,$(notdir $@) -lmpfr -lgmp
BASH_CFLAGS=-DHAVE_CONFIG_H -DSHELL -O2 -fwrapv -D_GNU_SOURCE -DRECYCLES_PIDS  -Ibash-headers-4.1.2-9.el6_2.x86_64 -Ibash-headers-4.1.2-9.el6_2.x86_64/include -Ibash-headers-4.1.2-9.el6_2.x86_64/lib -Ibash-headers-4.1.2-9.el6_2.x86_64/builtins

SPEC_CFLAGS = -fPIC -I. $(BASH_CFLAGS)

all : float_eval.so

%.o : %.c
	$(CC) $(SPEC_CFLAGS) $(CFLAGS) -c -o $@ $<

float_eval.so : float_eval.o bin_tree.o
float_eval.o : bin_tree.o float_eval.h
bin_tree.o : bin_tree.h

clean :
	rm -f *.o core.*

mrproper : clean
	rm -f float_eval float_eval.so

%.so : %.o
	$(CC) $(CFLAGS_SO) -o $@ $^
