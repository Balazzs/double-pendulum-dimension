CC=g++
CFLAGS=-std=c++17 -I. -Wall -lOpenCL
DEPS = CLO.hpp header.hpp Location.hpp AreaSelector.hpp
OBJ = main.cpp AreaSelector.cpp CLO.cpp error_map.cpp

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pendulum-dimension: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

debug: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS)