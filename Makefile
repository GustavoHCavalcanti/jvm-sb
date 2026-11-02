CXX = g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude
SOURCES := src/main.cpp src/classfile.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGET := jvm

.PHONY: all clean test sanitize
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@
src/%.o: src/%.cpp include/classfile.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf build
test: $(TARGET)
	bash tests/scripts/run.sh
sanitize: CXXFLAGS += -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer
sanitize: clean all
