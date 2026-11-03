#include "classfile.hpp"
#include <exception>
#include <iostream>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
namespace {
// O programa escreve em UTF-8, mas o console do Windows usa outra página de código por padrão.
// Sem console (saída redirecionada ou terminal mintty), as chamadas não têm efeito.
class ConsoleUtf8 {
public:
    ConsoleUtf8() : previous_(GetConsoleOutputCP()) { SetConsoleOutputCP(CP_UTF8); }
    ~ConsoleUtf8() {
        std::cout.flush();
        if (previous_) SetConsoleOutputCP(previous_);
    }
private:
    UINT previous_;
};
}
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    ConsoleUtf8 consoleUtf8;
#endif
    if (argc != 3 || (std::string(argv[1]) != "--viewer" && std::string(argv[1]) != "-viewer")) {
        std::cerr << "Uso: jvm --viewer arquivo.class\n"
                     "O modo --interpreter está reservado para uma etapa futura.\n";
        return 2;
    }
    try { jvm::ClassFile cf(argv[2]); cf.display(std::cout); }
    catch (const std::exception& e) { std::cerr << "Erro: " << e.what() << "\n"; return 1; }
    return 0;
}
