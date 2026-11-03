// Gera um .class de demonstração sem exigir Java nem Python: basta o compilador C++.
// O arquivo equivale à compilação de:
//
//   public class Exemplo {
//       public static final int VALOR = 42;
//       public static final long GRANDE = 1234567890123L;
//       public Exemplo() { super(); }
//       public static int soma(int a, int b) { return a + b; }
//       public static String saudacao() { return "Olá, mundo"; }
//   }
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

namespace {

class Bytes {
public:
    Bytes& u1(unsigned v) { data_.push_back(char(v & 0xff)); return *this; }
    Bytes& u2(unsigned v) { return u1(v >> 8).u1(v); }
    Bytes& u4(uint32_t v) { return u2(v >> 16).u2(v & 0xffff); }
    Bytes& u8(uint64_t v) { return u4(uint32_t(v >> 32)).u4(uint32_t(v & 0xffffffffu)); }
    Bytes& raw(const std::string& s) { data_ += s; return *this; }
    Bytes& raw(const Bytes& b) { data_ += b.data_; return *this; }
    uint32_t size() const { return uint32_t(data_.size()); }
    const std::string& str() const { return data_; }
private:
    std::string data_;
};

class Pool {
public:
    Pool() : next_(1) {}
    unsigned utf8(const std::string& s) { body_.u1(1).u2(unsigned(s.size())).raw(s); return next_++; }
    unsigned integer(uint32_t v) { body_.u1(3).u4(v); return next_++; }
    // Long ocupa dois índices da constant pool.
    unsigned longValue(uint64_t v) { body_.u1(5).u8(v); unsigned i = next_; next_ += 2; return i; }
    unsigned classRef(const std::string& name) { unsigned n = utf8(name); body_.u1(7).u2(n); return next_++; }
    unsigned stringRef(const std::string& s) { unsigned n = utf8(s); body_.u1(8).u2(n); return next_++; }
    unsigned methodRef(unsigned owner, const std::string& name, const std::string& descriptor) {
        unsigned n = utf8(name), d = utf8(descriptor);
        body_.u1(12).u2(n).u2(d);
        unsigned nameAndType = next_++;
        body_.u1(10).u2(owner).u2(nameAndType);
        return next_++;
    }
    unsigned count() const { return next_; }
    const Bytes& body() const { return body_; }
private:
    Bytes body_;
    unsigned next_;
};

Bytes attribute(unsigned name, const Bytes& info) {
    Bytes out;
    out.u2(name).u4(info.size()).raw(info);
    return out;
}

// Monta um método com Code e um LineNumberTable que associa o pc 0 à linha informada.
Bytes method(unsigned flags, unsigned name, unsigned descriptor, unsigned maxStack, unsigned maxLocals,
             const Bytes& code, unsigned line, unsigned codeName, unsigned linesName) {
    Bytes lines;
    lines.u2(1).u2(0).u2(line);
    Bytes info;
    info.u2(maxStack).u2(maxLocals).u4(code.size()).raw(code).u2(0).u2(1).raw(attribute(linesName, lines));
    Bytes out;
    out.u2(flags).u2(name).u2(descriptor).u2(1).raw(attribute(codeName, info));
    return out;
}

Bytes constantField(unsigned name, unsigned descriptor, unsigned constantValueName, unsigned value) {
    Bytes constant;
    constant.u2(value);
    Bytes out;
    out.u2(0x0019).u2(name).u2(descriptor).u2(1).raw(attribute(constantValueName, constant));
    return out;
}

}

int main(int argc, char** argv) {
    const std::string path = argc > 1 ? argv[1] : "Exemplo.class";
    Pool cp;
    const unsigned thisClass = cp.classRef("Exemplo");
    const unsigned superClass = cp.classRef("java/lang/Object");
    const unsigned objectInit = cp.methodRef(superClass, "<init>", "()V");
    const unsigned greeting = cp.stringRef("Ol\xC3\xA1, mundo");
    const unsigned codeName = cp.utf8("Code");
    const unsigned linesName = cp.utf8("LineNumberTable");
    const unsigned constantValueName = cp.utf8("ConstantValue");
    const unsigned sourceFileName = cp.utf8("SourceFile");
    const unsigned sourceFile = cp.utf8("Exemplo.java");
    const unsigned valorName = cp.utf8("VALOR"), intDescriptor = cp.utf8("I"), valor = cp.integer(42);
    const unsigned grandeName = cp.utf8("GRANDE"), longDescriptor = cp.utf8("J");
    const unsigned grande = cp.longValue(1234567890123ULL);
    const unsigned initName = cp.utf8("<init>"), initDescriptor = cp.utf8("()V");
    const unsigned somaName = cp.utf8("soma"), somaDescriptor = cp.utf8("(II)I");
    const unsigned saudacaoName = cp.utf8("saudacao"), saudacaoDescriptor = cp.utf8("()Ljava/lang/String;");

    Bytes initCode;
    initCode.u1(0x2a).u1(0xb7).u2(objectInit).u1(0xb1);              // aload_0, invokespecial, return
    Bytes somaCode;
    somaCode.u1(0x1a).u1(0x1b).u1(0x60).u1(0xac);                    // iload_0, iload_1, iadd, ireturn
    Bytes saudacaoCode;
    saudacaoCode.u1(0x12).u1(greeting).u1(0xb0);                     // ldc, areturn

    Bytes source;
    source.u2(sourceFile);

    Bytes file;
    file.u4(0xCAFEBABEu).u2(0).u2(52).u2(cp.count()).raw(cp.body());
    file.u2(0x0021).u2(thisClass).u2(superClass).u2(0);
    file.u2(2)
        .raw(constantField(valorName, intDescriptor, constantValueName, valor))
        .raw(constantField(grandeName, longDescriptor, constantValueName, grande));
    file.u2(3)
        .raw(method(0x0001, initName, initDescriptor, 1, 1, initCode, 3, codeName, linesName))
        .raw(method(0x0009, somaName, somaDescriptor, 2, 2, somaCode, 4, codeName, linesName))
        .raw(method(0x0009, saudacaoName, saudacaoDescriptor, 1, 0, saudacaoCode, 5, codeName, linesName));
    file.u2(1).raw(attribute(sourceFileName, source));

    std::ofstream out(path.c_str(), std::ios::binary | std::ios::trunc);
    out.write(file.str().data(), std::streamsize(file.str().size()));
    out.close();
    if (!out) {
        std::cerr << "Erro: não foi possível gravar " << path << "\n";
        return 1;
    }
    std::cout << "Gerado: " << path << "\n";
    return 0;
}
