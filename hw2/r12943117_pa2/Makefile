CC=g++
LDFLAGS=-std=c++11 -O3 -lm#-g
SOURCES=src/floorplanner.cpp src/main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=fp
INCLUDES=src/module.h src/floorplanner.h

all: $(SOURCES) $(INCLUDES) bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.cpp $(INCLUDES)
	$(CC) $(LDFLAGS) -c $< -o $@

clean:
	rm -rf src/*.o bin/$(EXECUTABLE)
