CXX=/usr/local/gcc-7.2.0/bin/g++
CC=$(CXX)
CXXFLAGS?=-std=c++17 -Wall -Werror --pedantic-errors -O0 -g
# CXXFLAGS?=-std=c++17 -Wall -Werror --pedantic-errors -O3 -lfto -DNDEBUG -march=native
# LDFLAGS?=-O3 -flto -march=native

Steiner: Steiner.o MST.o Net.o

Steiner.o: Steiner.cpp Net.h Types.h MST.h

MST.o: MST.cpp MST.h

Net.o : Net.h

clean:
	rm -rf *.o *~ Steiner
