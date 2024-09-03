CC=g++
CXXFLAGS=-std=c++17 -static -D_GLIBCXX_ISE_CXX11_ABI=1 -O2  # for release
# CXXFLAGS=-std=c++17 -g -static -Wall -D_GLIBCXX_ISE_CXX11_ABI=1  # for debug
LDFLAGS=-Llib -lDetailPlace -lGlobalPlace -lLegalizer -lPlacement -lParser -lPlaceCommon
SOURCES=src/ObjectiveFunction.cpp src/Optimizer.cpp src/GlobalPlacer.cpp src/partitioner.cpp  src/main.cpp
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=place

all: $(SOURCES) bin/$(EXECUTABLE)
	
bin/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(CXXFLAGS) $(LDFLAGS) -o $@

clean:
	rm -rf *.o bin/$(EXECUTABLE)