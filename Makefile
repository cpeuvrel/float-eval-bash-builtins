.PHONY: clean, mrproper
CC = gcc

CFLAGS_OPTI = 2
CFLAGS_BUILTIN = -D BASH_BUILTIN
CFLAGS = -W -Wall -O$(CFLAGS_OPTI) -std=c11 -lmpfr -lgmp $(CFLAGS_BUILTIN) $(CFLAGS_DBG)
CFLAGS_SO = -shared -O$(CFLAGS_OPTI) -Wl,-soname,$(notdir $@) -lmpfr -lgmp $(CFLAGS_BUILTIN)
BASH_CFLAGS = -fPIC -I. -DHAVE_CONFIG_H -DSHELL -fwrapv -D_GNU_SOURCE -DRECYCLES_PIDS  -Ibash-headers-4.1.2-9.el6_2.x86_64 -Ibash-headers-4.1.2-9.el6_2.x86_64/include -Ibash-headers-4.1.2-9.el6_2.x86_64/lib -Ibash-headers-4.1.2-9.el6_2.x86_64/builtins

ifdef static
	TO_BUILD = float_eval
	CFLAGS_BUILTIN =
else
	TO_BUILD = float_eval.so
endif

ifdef debug
	CFLAGS_OPTI = 0
	CFLAGS_DBG = -g -ggdb3 -Wno-unused-function -D DEBUG
endif

all : $(TO_BUILD)

float_eval: float_eval.o
	$(CC) $(CFLAGS) -o $@ $+

%.o : %.c
	$(CC) $(BASH_CFLAGS) $(CFLAGS) -c -o $@ $<

%.so : %.o
	$(CC) $(CFLAGS_SO) -o $@ $^

float_eval.so : float_eval.o
float_eval.o : float_eval.h

test: mrproper float_eval.so
	./test.sh

clean :
	rm -f *.o core.*

mrproper : clean
	rm -f float_eval float_eval.so
