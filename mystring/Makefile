CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -O2

main: main.cpp MyString.h
	$(CXX) $(CXXFLAGS) main.cpp -o $@

test: main
	./main

clean:
	rm -f main

.PHONY: test clean
