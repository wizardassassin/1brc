CXX = g++
CXXFLAGS = -std=c++23 -Ofast -march=native -flto=auto -funroll-loops

.PHONY: all release

all: main.out main2.out

release: all

%.out: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f main.out main2.out
