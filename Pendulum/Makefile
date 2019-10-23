CC=g++
CFLAGS=-std=c++17 -I. -Wall -lOpenCL
DEPS = CLO.hpp header.hpp Location.hpp SimQueue.hpp DataCollector.hpp AreaSelector.hpp
OBJ = CLO.cpp error_map.cpp SimQueue.cpp DataCollector.cpp AreaSelector.cpp

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pendulum-dimension: $(OBJ)
	$(CC) -o $@ piped_run.cpp $^ $(CFLAGS)

pendulum-auto: $(OBJ)
	$(CC) -o $@ program.cpp $^ $(CFLAGS)

debug: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS)

clean:
	rm pendulum-dimension
	rm debug