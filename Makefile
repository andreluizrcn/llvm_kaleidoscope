# Compiler and Linker
CXX = clang++
CXXFLAGS = `llvm-config --cxxflags` -std=c++17 -g -O0
LDFLAGS = `llvm-config --ldflags`
LIBS = `llvm-config --libs`

# Target executable for Chapter 1
TARGET = kaleidoscope-ch1

# Source files (adicione os .cpp conforme for avançando)
SRCS = chapter1/lexer.cpp chapter1/main.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
