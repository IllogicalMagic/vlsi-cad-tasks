CXX=/usr/local/gcc-7.2.0/bin/g++
CC=$(CXX)
CXXFLAGS?=-std=c++17 -Wall -Werror --pedantic-errors -O0 -g

Steiner: Steiner.o MST.o

Steiner.o: Steiner.cpp Net.h Types.h MST.h

MST.o: MST.cpp MST.h

clean:
	rm -rf *.o *~ Steiner
