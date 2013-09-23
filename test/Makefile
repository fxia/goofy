#-*- mode: makefile -*-

OBJS = timer_test.o

%.o : %.c

timer_test : timer_test.o ../libgoofy.so
	gcc -o $@ $< -L../ -Wl,--rpath=../ -lgoofy -lstdc++

clean :
	rm -rf timer_test *.o
