CXX = g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude
LDFLAGS ?=
SOURCES := src/main.cpp src/classfile.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGET := jvm

# MinGW-w64 (MSYS2 UCRT64 ou compilação cruzada): o g++ gera .exe, e -static
# permite executá-lo fora do terminal UCRT64, sem as DLLs do GCC no PATH.
ifneq (,$(findstring mingw,$(shell $(CXX) -dumpmachine)))
TARGET := jvm.exe
LDFLAGS += -static
endif

FUZZ_CXX ?= clang++
FUZZ_TARGET := build/fuzz_classfile
FUZZ_SEEDS := build/fuzz-corpus

.PHONY: all clean test exemplo sanitize cppcheck fuzz-build fuzz-smoke demo
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) -o $@
src/%.o: src/%.cpp include/classfile.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJECTS) jvm jvm.exe
	rm -rf build
test: $(TARGET)
	bash tests/scripts/run.sh
exemplo: build/Exemplo.class
build/Exemplo.class: examples/gera_exemplo.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) examples/gera_exemplo.cpp $(LDFLAGS) -o build/gera_exemplo
	build/gera_exemplo $@
demo: $(TARGET)
	javac --release 8 -d demo demo/Ola.java
	./$(TARGET) --viewer demo/Ola.class
sanitize: CXXFLAGS += -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer
sanitize: clean all
cppcheck:
	@command -v cppcheck >/dev/null || { echo "cppcheck não encontrado; instale com: brew install cppcheck"; exit 1; }
	cppcheck --language=c++ --std=c++11 --enable=warning,performance,portability -Iinclude src tests/fuzz
fuzz-build: $(FUZZ_TARGET)
$(FUZZ_TARGET): tests/fuzz/fuzz_classfile.cpp src/classfile.cpp include/classfile.hpp
	mkdir -p build
	$(FUZZ_CXX) -std=c++11 -g -O1 -fno-omit-frame-pointer -fsanitize=fuzzer,address -Iinclude tests/fuzz/fuzz_classfile.cpp src/classfile.cpp -o $@
fuzz-smoke: $(FUZZ_TARGET)
	mkdir -p $(FUZZ_SEEDS) build/fuzz-artifacts
	python3 tests/scripts/make_minimal.py $(FUZZ_SEEDS)/Minimal.class
	python3 tests/scripts/make_attribute_fixture.py $(FUZZ_SEEDS)/Attributes.class
	python3 tests/scripts/make_bytecode_fixture.py $(FUZZ_SEEDS)/Bytecodes.class
	$(FUZZ_TARGET) -max_total_time=10 -timeout=5 -max_len=65536 -artifact_prefix=build/fuzz-artifacts/ $(FUZZ_SEEDS)
