.PHONY: clean, mrproper
CC = gcc
CFLAGS = -g -W -Wall
BASH_CFLAGS=-DHAVE_CONFIG_H -DSHELL -O2 -g -fwrapv -D_GNU_SOURCE -DRECYCLES_PIDS  -Ibash-headers-4.1.2-9.el6_2.x86_64 -Ibash-headers-4.1.2-9.el6_2.x86_64/include -Ibash-headers-4.1.2-9.el6_2.x86_64/lib -Ibash-headers-4.1.2-9.el6_2.x86_64/builtins

SPEC_CFLAGS = -fPIC -I. $(BASH_CFLAGS)

all : float_eval.so

%.o : %.c
	$(CC) $(SPEC_CFLAGS) $(CFLAGS) -c -o $@ $<

float_eval.so : float_eval.o binTree.o
binTree.o : binTree.h

clean :
	rm -f *.o core.*

mrproper : clean
	rm -f float_eval float_eval.so

%.so : %.o
	$(CC) -shared -Wl,-soname,$(notdir $@) -o $@ $^
