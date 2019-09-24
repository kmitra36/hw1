# cs335 lab2
# to compile your project, type make and press enter

all: lab2

lab2: lab2.cpp
	g++ $(CFLAGS) lab2.cpp libggfonts.a -Wall -olab2 -pthread -lX11 -lGL -lGLU -lm

clean:
	rm -f lab2
	rm -f *.o
