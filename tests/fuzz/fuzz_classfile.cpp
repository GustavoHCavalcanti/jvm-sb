// Fuzz target do leitor: ClassFile só lê de um caminho, então cada entrada passa por um arquivo temporário.
#include "classfile.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    static const std::string path = "/tmp/jvm_fuzz_" + std::to_string(getpid()) + ".class";
    {
        std::ofstream f(path.c_str(), std::ios::binary | std::ios::trunc);
        f.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    }
    // Rejeitar entrada inválida com exceção é o comportamento esperado; crash do sanitizer é o defeito procurado.
    try {
        jvm::ClassFile cf(path);
        std::ostringstream out;
        cf.display(out);
    } catch (const std::exception&) {
    }
    return 0;
}
