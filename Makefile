.PHONY: clean, mrproper
CC = gcc
CFLAGS = -g -W -Wall -std=c11 -lmpfr -lgmp -D BASH_BUILTIN
CFLAGS_SO = -shared -Wl,-soname,$(notdir $@) -lmpfr -lgmp -D BASH_BUILTIN
BASH_CFLAGS=-DHAVE_CONFIG_H -DSHELL -O2 -fwrapv -D_GNU_SOURCE -DRECYCLES_PIDS  -Ibash-headers-4.1.2-9.el6_2.x86_64 -Ibash-headers-4.1.2-9.el6_2.x86_64/include -Ibash-headers-4.1.2-9.el6_2.x86_64/lib -Ibash-headers-4.1.2-9.el6_2.x86_64/builtins

SPEC_CFLAGS = -fPIC -I. $(BASH_CFLAGS)

all : float_eval.so

float_eval: float_eval.o
	$(CC) $(CFLAGS) -o $@ $+

%.o : %.c
	$(CC) $(SPEC_CFLAGS) $(CFLAGS) -c -o $@ $<

float_eval.so : float_eval.o
float_eval.o : float_eval.h

clean :
	rm -f *.o core.*

mrproper : clean
	rm -f float_eval float_eval.so

%.so : %.o
	$(CC) $(CFLAGS_SO) -o $@ $^
