#-*- mode: makefile -*-

HDRS = Util.h Timer.h Packet.h Dispatcher.h
OBJS = Util.o Timer.o Packet.o Dispatcher.o

all : libgoofy.so

libgoofy.so : $(OBJS)
	gcc -shared -o $@ $(OBJS) -lstdc++

# No deps yet
%.o : %.cc $(HDRS) 
	g++ -c -fPIC -Wall $<

clean :
	rm -rf libgoofy.so $(OBJS)
