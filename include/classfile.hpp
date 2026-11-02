#ifndef JVM_CLASSFILE_HPP
#define JVM_CLASSFILE_HPP

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace jvm {

struct CpEntry {
    uint8_t tag;
    uint16_t a, b;
    uint32_t bits;
    uint64_t wide;
    std::string text;
    CpEntry() : tag(0), a(0), b(0), bits(0), wide(0) {}
};

struct Attribute {
    uint16_t nameIndex;
    std::string name;
    std::vector<uint8_t> info;
};
struct Member {
    uint16_t flags, nameIndex, descriptorIndex;
    std::vector<Attribute> attributes;
};

class ClassFile {
public:
    explicit ClassFile(const std::string& path);
    void display(std::ostream& out) const;
    const std::vector<CpEntry>& pool() const { return cp_; }
    std::string cpDescription(uint16_t index) const;
    std::string utf8(uint16_t index) const;
private:
    std::vector<uint8_t> data_;
    std::vector<CpEntry> cp_;
    uint16_t minor_, major_, flags_, this_, super_;
    std::vector<uint16_t> interfaces_;
    std::vector<Member> fields_, methods_;
    std::vector<Attribute> attributes_;
    void parse();
    void printPool(std::ostream&) const;
    void printMember(std::ostream&, const Member&, bool) const;
    void printAttributes(std::ostream&, const std::vector<Attribute>&, unsigned) const;
    void printCode(std::ostream&, const Attribute&, unsigned) const;
};

} // namespace jvm
#endif
