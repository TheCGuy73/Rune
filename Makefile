# Variabili
CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17
SRC = $(wildcard src/*.cpp)
PARSER = $(wildcard parser/*.cpp)
OBJ = $(SRC:.cpp=.o) $(PARSER:.cpp=.o)
TARGET = bin/voltage

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

parser/%.o: parser/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf src/*.o parser/*.o bin/

.PHONY: all clean