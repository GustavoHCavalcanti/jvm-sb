#include "classfile.hpp"
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 3 || (std::string(argv[1]) != "--viewer" && std::string(argv[1]) != "-viewer")) {
        std::cerr << "Uso: jvm --viewer arquivo.class\n"
                     "O modo --interpreter está reservado para uma etapa futura.\n";
        return 2;
    }
    try { jvm::ClassFile cf(argv[2]); cf.display(std::cout); }
    catch (const std::exception& e) { std::cerr << "Erro: " << e.what() << "\n"; return 1; }
    return 0;
}
