#-*- mode: makefile -*-

OBJS = Util.o Timer.o Packet.o

all : libgoofy.so

libgoofy.so : $(OBJS)
	gcc -shared -o $@ $(OBJS) -lstdc++

%.o : %.cc %.h
	g++ -c -fPIC -Wall $<

clean :
	rm -rf libgoofy.so $(OBJS)
