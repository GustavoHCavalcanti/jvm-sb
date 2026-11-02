#include "classfile.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace jvm {
namespace {
class Reader {
public:
    Reader(const std::vector<uint8_t>& d, size_t p=0, size_t n=std::numeric_limits<size_t>::max())
        : d_(d), p_(p), end_(std::min(d.size(), n > d.size()-std::min(p,d.size()) ? d.size() : p+n)) {
        if (p>d.size()) throw std::runtime_error("offset fora do arquivo");
    }
    size_t pos() const { return p_; }
    size_t remaining() const { return end_-p_; }
    uint8_t u1(){ need(1); return d_[p_++]; }
    uint16_t u2(){ return (uint16_t(u1())<<8)|u1(); }
    uint32_t u4(){ return (uint32_t(u2())<<16)|u2(); }
    uint64_t u8(){ return (uint64_t(u4())<<32)|u4(); }
    std::vector<uint8_t> bytes(size_t n){ need(n); std::vector<uint8_t> v(d_.begin()+p_,d_.begin()+p_+n); p_+=n; return v; }
    void skip(size_t n){ need(n); p_+=n; }
private:
    const std::vector<uint8_t>& d_; size_t p_,end_;
    void need(size_t n){ if(n>remaining()) throw std::runtime_error("arquivo truncado ou estrutura fora dos limites"); }
};
std::string hex(uint64_t v, unsigned w=0){ std::ostringstream s; s<<"0x"<<std::hex<<std::uppercase<<std::setw(w)<<std::setfill('0')<<v; return s.str(); }
std::string quoted(const std::string& value){
    std::ostringstream out;out<<'"';
    for(unsigned char c:value){
        if(c=='\\')out<<"\\\\";else if(c=='"')out<<"\\\"";else if(c=='\n')out<<"\\n";else if(c=='\r')out<<"\\r";else if(c=='\t')out<<"\\t";else if(c==0)out<<"\\u0000";else if(c<0x20||c==0x7f)out<<"\\x"<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(c)<<std::dec;else out<<char(c);
    }
    out<<'"';return out.str();
}
std::string access(uint16_t f, int kind){
    struct Flag{uint16_t b; const char*n;};
    static const Flag cls[]={{1,"public"},{0x10,"final"},{0x20,"super"},{0x200,"interface"},{0x400,"abstract"},{0x1000,"synthetic"},{0x2000,"annotation"},{0x4000,"enum"}};
    static const Flag fld[]={{1,"public"},{2,"private"},{4,"protected"},{8,"static"},{0x10,"final"},{0x40,"volatile"},{0x80,"transient"},{0x1000,"synthetic"},{0x4000,"enum"}};
    static const Flag met[]={{1,"public"},{2,"private"},{4,"protected"},{8,"static"},{0x10,"final"},{0x20,"synchronized"},{0x40,"bridge"},{0x80,"varargs"},{0x100,"native"},{0x400,"abstract"},{0x800,"strict"},{0x1000,"synthetic"}};
    const Flag* a=kind==0?cls:(kind==1?fld:met); size_t n=kind==0?8:(kind==1?9:12); std::ostringstream o; bool first=true;
    for(size_t i=0;i<n;i++) {
        if(f&a[i].b){ if(!first)o<<' '; o<<a[i].n; first=false; }
    }
    return first?"(none)":o.str();
}
const char* tagName(unsigned t){ switch(t){case 1:return"Utf8";case 3:return"Integer";case 4:return"Float";case 5:return"Long";case 6:return"Double";case 7:return"Class";case 8:return"String";case 9:return"Fieldref";case 10:return"Methodref";case 11:return"InterfaceMethodref";case 12:return"NameAndType";case 15:return"MethodHandle";case 16:return"MethodType";case 18:return"InvokeDynamic";default:return"Invalid";} }
void appendUtf8(std::string& out, uint32_t cp) {
    if(cp<=0x7f) out.push_back(char(cp));
    else if(cp<=0x7ff){out.push_back(char(0xc0|(cp>>6)));out.push_back(char(0x80|(cp&0x3f)));}
    else if(cp<=0xffff){out.push_back(char(0xe0|(cp>>12)));out.push_back(char(0x80|((cp>>6)&0x3f)));out.push_back(char(0x80|(cp&0x3f)));}
    else {out.push_back(char(0xf0|(cp>>18)));out.push_back(char(0x80|((cp>>12)&0x3f)));out.push_back(char(0x80|((cp>>6)&0x3f)));out.push_back(char(0x80|(cp&0x3f)));}
}
std::string decodeModifiedUtf8(const std::vector<uint8_t>& b) {
    std::vector<uint16_t> units;
    for(size_t i=0;i<b.size();){uint8_t x=b[i++];uint16_t c;
        if(x>=1&&x<=0x7f)c=x;
        else if((x&0xe0)==0xc0){if(i>=b.size()||(b[i]&0xc0)!=0x80)throw std::runtime_error("Modified UTF-8 inválido");uint8_t y=b[i++];c=uint16_t(((x&0x1f)<<6)|(y&0x3f));if(c!=0&&c<0x80)throw std::runtime_error("Modified UTF-8 não canônico");}
        else if((x&0xf0)==0xe0){if(i+1>=b.size()||(b[i]&0xc0)!=0x80||(b[i+1]&0xc0)!=0x80)throw std::runtime_error("Modified UTF-8 inválido");uint8_t y=b[i++],z=b[i++];c=uint16_t(((x&0x0f)<<12)|((y&0x3f)<<6)|(z&0x3f));if(c<0x800)throw std::runtime_error("Modified UTF-8 não canônico");}
        else throw std::runtime_error("byte inválido em Modified UTF-8");
        units.push_back(c);
    }
    std::string out;
    for(size_t i=0;i<units.size();i++){uint32_t c=units[i];if(c>=0xd800&&c<=0xdbff){if(i+1>=units.size()||units[i+1]<0xdc00||units[i+1]>0xdfff)throw std::runtime_error("par surrogate inválido em Modified UTF-8");c=0x10000+((c-0xd800)<<10)+(units[++i]-0xdc00);}else if(c>=0xdc00&&c<=0xdfff)throw std::runtime_error("par surrogate inválido em Modified UTF-8");appendUtf8(out,c);}
    return out;
}
// Java 8 opcode names, indexed by opcode (reserved values are marked invalid).
const char* opnames[256]={
"nop","aconst_null","iconst_m1","iconst_0","iconst_1","iconst_2","iconst_3","iconst_4","iconst_5","lconst_0","lconst_1","fconst_0","fconst_1","fconst_2","dconst_0","dconst_1","bipush","sipush","ldc","ldc_w","ldc2_w","iload","lload","fload","dload","aload","iload_0","iload_1","iload_2","iload_3","lload_0","lload_1","lload_2","lload_3","fload_0","fload_1","fload_2","fload_3","dload_0","dload_1","dload_2","dload_3","aload_0","aload_1","aload_2","aload_3","iaload","laload","faload","daload","aaload","baload","caload","saload","istore","lstore","fstore","dstore","astore","istore_0","istore_1","istore_2","istore_3","lstore_0","lstore_1","lstore_2","lstore_3","fstore_0","fstore_1","fstore_2","fstore_3","dstore_0","dstore_1","dstore_2","dstore_3","astore_0","astore_1","astore_2","astore_3","iastore","lastore","fastore","dastore","aastore","bastore","castore","sastore","pop","pop2","dup","dup_x1","dup_x2","dup2","dup2_x1","dup2_x2","swap","iadd","ladd","fadd","dadd","isub","lsub","fsub","dsub","imul","lmul","fmul","dmul","idiv","ldiv","fdiv","ddiv","irem","lrem","frem","drem","ineg","lneg","fneg","dneg","ishl","lshl","ishr","lshr","iushr","lushr","iand","land","ior","lor","ixor","lxor","iinc","i2l","i2f","i2d","l2i","l2f","l2d","f2i","f2l","f2d","d2i","d2l","d2f","i2b","i2c","i2s","lcmp","fcmpl","fcmpg","dcmpl","dcmpg","ifeq","ifne","iflt","ifge","ifgt","ifle","if_icmpeq","if_icmpne","if_icmplt","if_icmpge","if_icmpgt","if_icmple","if_acmpeq","if_acmpne","goto","jsr","ret","tableswitch","lookupswitch","ireturn","lreturn","freturn","dreturn","areturn","return","getstatic","putstatic","getfield","putfield","invokevirtual","invokespecial","invokestatic","invokeinterface","invokedynamic","new","newarray","anewarray","arraylength","athrow","checkcast","instanceof","monitorenter","monitorexit","wide","multianewarray","ifnull","ifnonnull","goto_w","jsr_w"};
bool validOp(uint8_t op){return op<=0xc9 && !(op>=0xca);}
void disassemble(std::ostream& out,const std::vector<uint8_t>& code,const ClassFile& cf,unsigned indent){
    size_t p=0; std::string pad(indent,' ');
    while(p<code.size()){
        size_t start=p; uint8_t op=code[p++]; if(!validOp(op)){out<<pad<<start<<": <invalid opcode "<<hex(op,2)<<">\n";break;}
        out<<pad<<start<<": "<<opnames[op]; auto u1=[&](){if(p>=code.size())throw std::runtime_error("operando truncado");return code[p++];}; auto u2=[&](){uint16_t x=uint16_t(u1())<<8;return uint16_t(x|u1());}; auto s2=[&](){return int16_t(u2());}; auto s4=[&](){uint32_t x=uint32_t(u2())<<16; x|=u2(); return int32_t(x);};
        try {
            if(op==0xaa||op==0xab){ while(p%4)u1(); int32_t def=s4(); out<<" default:"<<int64_t(start)+def; if(op==0xaa){int32_t lo=s4(),hi=s4(); if(hi<lo||uint64_t(int64_t(hi)-int64_t(lo))>code.size())throw std::runtime_error("tableswitch inválido"); out<<" low:"<<lo<<" high:"<<hi; for(int64_t k=lo;k<=hi;k++)out<<" "<<k<<":"<<int64_t(start)+s4();}else{int32_t n=s4();if(n<0||uint32_t(n)>code.size()/8)throw std::runtime_error("lookupswitch inválido");int32_t previous=std::numeric_limits<int32_t>::min();for(int32_t k=0;k<n;k++){int32_t key=s4(),offset=s4();if(k&&key<=previous)throw std::runtime_error("lookupswitch keys fora de ordem");previous=key;out<<" "<<key<<":"<<int64_t(start)+offset;}}}
            else if(op==0xc4){uint8_t x=u1();if(!((x>=0x15&&x<=0x19)||(x>=0x36&&x<=0x3a)||x==0x84||x==0xa9))throw std::runtime_error("subopcode wide inválido");out<<" "<<opnames[x]<<" "<<u2();if(x==0x84)out<<" "<<int16_t(u2());}
            else if(op==0x10)out<<" "<<int(int8_t(u1())); else if(op==0x11)out<<" "<<s2();
            else if(op==0x12||op==0x13||op==0x14|| (op>=0xb2&&op<=0xb8)||op==0xbb||op==0xbd||op==0xc0||op==0xc1||op==0xb9||op==0xba||op==0xc5){uint16_t idx=op==0x12?u1():u2();out<<" #"<<idx<<" // "<<cf.cpDescription(idx);if(op==0xb9)out<<" count/zero="<<unsigned(u1())<<"/"<<unsigned(u1());else if(op==0xba)out<<" zero="<<unsigned(u1())<<"/"<<unsigned(u1()); if(op==0xc5)out<<" dimensions="<<unsigned(u1());}
            else if(op==0x84)out<<" local="<<unsigned(u1())<<" increment="<<int(int8_t(u1()));
            else if((op>=0x15&&op<=0x19)||(op>=0x36&&op<=0x3a)||op==0xa9)out<<" "<<unsigned(u1());
            else if(op==0xbc){uint8_t t=u1();static const char* types[]={"invalid","invalid","invalid","invalid","boolean","char","float","double","byte","short","int","long"};if(t<4||t>11)throw std::runtime_error("newarray atype inválido");out<<" "<<unsigned(t)<<" // "<<types[t];}
            else if((op>=0x99&&op<=0xa8)||op==0xc6||op==0xc7)out<<" "<<int64_t(start)+s2();
            else if(op==0xc8||op==0xc9)out<<" "<<int64_t(start)+s4();
            else if(op==0x16||op==0x17||op==0x18||op==0x19||op==0x36||op==0x37||op==0x38||op==0x39||op==0x3a){} 
        } catch(const std::exception& e){out<<" <"<<e.what()<<">\n"; return;}
        out<<"\n";
    }
}
void checkEnd(const Reader& r, const std::string& name) {
    if(r.remaining()) throw std::runtime_error("comprimento inválido no atributo "+name);
}
void verificationType(Reader& r, std::ostream& o, const ClassFile& cf) {
    uint8_t t=r.u1(); static const char* names[]={"Top","Integer","Float","Double","Long","Null","UninitializedThis"};
    if(t<=6)o<<names[t]; else if(t==7){uint16_t i=r.u2();o<<"Object #"<<i<<" // "<<cf.cpDescription(i);}else if(t==8)o<<"Uninitialized offset="<<r.u2();else throw std::runtime_error("verification_type_info inválido");
}
void annotation(Reader& r, std::ostream& o, const ClassFile& cf, unsigned depth);
void elementValue(Reader& r, std::ostream& o, const ClassFile& cf, unsigned depth) {
    if(depth>64)throw std::runtime_error("anotação excessivamente aninhada");
    uint8_t tag=r.u1();o<<char(tag)<<' ';
    if(std::string("BCDFIJSZs").find(char(tag))!=std::string::npos){uint16_t i=r.u2();o<<"#"<<i<<" // "<<cf.cpDescription(i);}
    else if(tag=='e'){uint16_t t=r.u2(),n=r.u2();o<<"enum #"<<t<<"."<<cf.cpDescription(t)<<" #"<<n<<"."<<cf.cpDescription(n);}
    else if(tag=='c'){uint16_t i=r.u2();o<<"class #"<<i<<" // "<<cf.cpDescription(i);}
    else if(tag=='@')annotation(r,o,cf,depth+1);
    else if(tag=='['){uint16_t n=r.u2();o<<"["<<n<<"]";for(unsigned i=0;i<n;i++){o<<"\n"<<std::string(depth+2,' ');elementValue(r,o,cf,depth+1);}}
    else throw std::runtime_error("element_value tag inválida");
}
void annotation(Reader& r, std::ostream& o, const ClassFile& cf, unsigned depth) {
    if(depth>64)throw std::runtime_error("anotação excessivamente aninhada");
    uint16_t ti=r.u2(),n=r.u2();o<<"type #"<<ti<<" // "<<cf.cpDescription(ti)<<" pairs="<<n;
    for(unsigned i=0;i<n;i++){uint16_t ni=r.u2();o<<"\n"<<std::string(depth+2,' ')<<"element #"<<ni<<" // "<<cf.cpDescription(ni)<<" = ";elementValue(r,o,cf,depth+1);}
}
void annotations(Reader& r, std::ostream& o, const ClassFile& cf, unsigned depth) {
    uint16_t n=r.u2();o<<" count="<<n;
    for(unsigned i=0;i<n;i++){o<<"\n"<<std::string(depth+2,' ')<<"annotation ";annotation(r,o,cf,depth+1);}
}
void typeAnnotation(Reader& r, std::ostream& o, const ClassFile& cf, unsigned depth) {
    uint8_t target=r.u1();o<<"target="<<hex(target,2)<<" info=";
    switch(target){
      case 0x00:case 0x01:case 0x16:o<<unsigned(r.u1());break;
      case 0x10:case 0x17:case 0x42:case 0x43:case 0x44:case 0x45:case 0x46:o<<r.u2();break;
      case 0x11:case 0x12:o<<unsigned(r.u1())<<","<<unsigned(r.u1());break;
      case 0x40:case 0x41:{uint16_t n=r.u2();o<<"localvar["<<n<<"]";for(unsigned i=0;i<n;i++)o<<" {start="<<r.u2()<<",len="<<r.u2()<<",slot="<<r.u2()<<"}";break;}
      case 0x47:case 0x48:case 0x49:case 0x4a:case 0x4b:o<<r.u2()<<","<<unsigned(r.u1());break;
      default: if(target>=0x13&&target<=0x15)break; throw std::runtime_error("target_type de type annotation inválido");
    }
    uint8_t path=r.u1();o<<" path=";for(unsigned i=0;i<path;i++){uint8_t kind=r.u1(),arg=r.u1();o<<unsigned(kind)<<":"<<unsigned(arg)<<"/";}
    o<<" annotation ";annotation(r,o,cf,depth+1);
}
}

ClassFile::ClassFile(const std::string& path):minor_(0),major_(0),flags_(0),this_(0),super_(0){
    std::ifstream f(path.c_str(),std::ios::binary); if(!f)throw std::runtime_error("não foi possível abrir: "+path);
    f.seekg(0,std::ios::end); std::streamoff n=f.tellg(); if(n<0)throw std::runtime_error("não foi possível medir o arquivo");
    if(uint64_t(n)>512ull*1024*1024)throw std::runtime_error("arquivo excessivamente grande (>512 MiB)");
    data_.resize(size_t(n)); f.seekg(0); if(n && !f.read(reinterpret_cast<char*>(&data_[0]),n))throw std::runtime_error("falha ao ler arquivo"); parse();
}
void ClassFile::parse(){
    Reader r(data_); uint32_t magic=r.u4();if(magic!=0xcafebabe)throw std::runtime_error("assinatura inválida; esperado CAFEBABE"); minor_=r.u2();major_=r.u2();uint16_t count=r.u2();if(!count)throw std::runtime_error("constant_pool_count inválido"); cp_.resize(count);
    for(uint16_t i=1;i<count;i++){CpEntry&e=cp_[i];e.tag=r.u1();switch(e.tag){case 1:{uint16_t n=r.u2();e.text=decodeModifiedUtf8(r.bytes(n));break;}
        case 3:case 4:e.bits=r.u4();break;case 5:case 6:e.wide=r.u8();if(i+1>=count)throw std::runtime_error("Long/Double sem slot de continuação");i++;break;
        case 7:case 8:case 16:e.a=r.u2();break;case 9:case 10:case 11:case 12:case 18:e.a=r.u2();e.b=r.u2();break;case 15:e.a=r.u1();e.b=r.u2();break;default:throw std::runtime_error("tag constant pool desconhecida: "+std::to_string(e.tag));}}
    for(size_t i=1;i<cp_.size();i++){const CpEntry&e=cp_[i];auto require=[&](uint16_t x,uint8_t tag){if(!x||x>=cp_.size()||cp_[x].tag!=tag)throw std::runtime_error("referência inválida na constant pool #"+std::to_string(i));};switch(e.tag){case 7:case 8:case 16:require(e.a,1);break;case 9:case 10:case 11:require(e.a,7);require(e.b,12);break;case 12:require(e.a,1);require(e.b,1);break;case 15:if(e.a<1||e.a>9||!e.b||e.b>=cp_.size())throw std::runtime_error("MethodHandle inválido na constant pool");break;case 18:require(e.b,12);break;default:break;}}
    flags_=r.u2();this_=r.u2();super_=r.u2();uint16_t n=r.u2();for(uint16_t i=0;i<n;i++)interfaces_.push_back(r.u2());
    auto readMembers=[&](std::vector<Member>&v){uint16_t m=r.u2();for(uint16_t i=0;i<m;i++){Member x;x.flags=r.u2();x.nameIndex=r.u2();x.descriptorIndex=r.u2();uint16_t na=r.u2();for(uint16_t j=0;j<na;j++){Attribute a;a.nameIndex=r.u2();uint32_t len=r.u4();if(len>r.remaining())throw std::runtime_error("attribute_length excede o arquivo");a.name=utf8(a.nameIndex);a.info=r.bytes(len);x.attributes.push_back(a);}v.push_back(x);}};
    readMembers(fields_);readMembers(methods_);n=r.u2();for(uint16_t i=0;i<n;i++){Attribute a;a.nameIndex=r.u2();uint32_t len=r.u4();if(len>r.remaining())throw std::runtime_error("attribute_length excede o arquivo");a.name=utf8(a.nameIndex);a.info=r.bytes(len);attributes_.push_back(a);}if(r.remaining())throw std::runtime_error("bytes excedentes após ClassFile");
    auto classIndex=[&](uint16_t x){if(!x||x>=cp_.size()||cp_[x].tag!=7)throw std::runtime_error("índice de classe inválido no ClassFile");};
    classIndex(this_);if(super_)classIndex(super_);for(auto x:interfaces_)classIndex(x);
    for(const auto* list:{&fields_,&methods_})for(const Member&m:*list){(void)utf8(m.nameIndex);(void)utf8(m.descriptorIndex);}
}
std::string ClassFile::utf8(uint16_t i)const{if(!i||i>=cp_.size()||cp_[i].tag!=1)throw std::runtime_error("índice #"+std::to_string(i)+" não aponta para Utf8");return cp_[i].text;}
std::string ClassFile::cpDescription(uint16_t i)const{
    if(!i||i>=cp_.size()||cp_[i].tag==0) return "<índice inválido #"+std::to_string(i)+">";
    const CpEntry&e=cp_[i];std::ostringstream o;
    switch(e.tag){case 1:return e.text;case 3:{int32_t x=e.bits<=uint32_t(INT32_MAX)?int32_t(e.bits):int32_t(-1-int32_t(~e.bits));return std::to_string(x);}case 4:{float f;std::memcpy(&f,&e.bits,4);o<<std::setprecision(std::numeric_limits<float>::max_digits10)<<f;return o.str();}case 5:{int64_t x=e.wide<=uint64_t(INT64_MAX)?int64_t(e.wide):-1-int64_t(~e.wide);return std::to_string(x);}case 6:{double d;std::memcpy(&d,&e.wide,8);o<<std::setprecision(std::numeric_limits<double>::max_digits10)<<d;return o.str();}case 7:return cpDescription(e.a);case 8:return quoted(cpDescription(e.a));case 9:case 10:case 11:{std::string type=cpDescription(e.a);const CpEntry&nt=cp_[e.b];if(nt.tag!=12)return type+".#"+std::to_string(e.b);return type+"."+cpDescription(nt.a)+":"+cpDescription(nt.b);}case 12:return cpDescription(e.a)+":"+cpDescription(e.b);case 15:return "kind="+std::to_string(e.a)+" #"+std::to_string(e.b)+" "+cpDescription(e.b);case 16:return cpDescription(e.a);case 18:return "bootstrap="+std::to_string(e.a)+" "+cpDescription(e.b);default:return "<invalid>";}
}
void ClassFile::printPool(std::ostream&o)const{for(size_t i=1;i<cp_.size();i++){const CpEntry&e=cp_[i];o<<"  #"<<i<<" = ";if(!e.tag){o<<"<second slot>\n";continue;}o<<tagName(e.tag)<<' ';
    if(e.tag==1)o<<quoted(e.text);else if(e.tag==3||e.tag==4||e.tag==5||e.tag==6)o<<cpDescription(i)<<" ("<<hex(e.tag==5||e.tag==6?e.wide:e.bits,e.tag==5||e.tag==6?16:8)<<")";else if(e.tag==7||e.tag==8||e.tag==16)o<<'#'<<e.a<<" // "<<cpDescription(i);else if(e.tag==12)o<<'#'<<e.a<<":#"<<e.b<<" // "<<cpDescription(i);else if(e.tag==9||e.tag==10||e.tag==11||e.tag==18)o<<'#'<<e.a<<".#"<<e.b<<" // "<<cpDescription(i);else if(e.tag==15)o<<"kind="<<e.a<<" #"<<e.b<<" // "<<cpDescription(i);o<<"\n";}}
void ClassFile::printAttributes(std::ostream&o,const std::vector<Attribute>&as,unsigned depth)const{
    std::string p(depth,' ');for(const auto&a:as){o<<p<<a.name<<" (#"<<a.nameIndex<<", length="<<a.info.size()<<")";
        Reader r(a.info);
        if(a.name=="ConstantValue"||a.name=="SourceFile"||a.name=="Signature"){uint16_t i=r.u2();o<<" #"<<i<<" // "<<cpDescription(i);}
        else if(a.name=="Exceptions"){uint16_t n=r.u2();o<<" count="<<n<<"\n";for(unsigned i=0;i<n;i++){uint16_t x=r.u2();o<<p<<"  #"<<x<<" // "<<cpDescription(x)<<"\n";}checkEnd(r,a.name);continue;}
        else if(a.name=="Code"){o<<"\n";printCode(o,a,depth+2);continue;}
        else if(a.name=="LineNumberTable"){uint16_t n=r.u2();o<<" entries="<<n<<"\n";for(unsigned i=0;i<n;i++)o<<p<<"  pc="<<r.u2()<<" line="<<r.u2()<<"\n";checkEnd(r,a.name);continue;}
        else if(a.name=="LocalVariableTable"||a.name=="LocalVariableTypeTable"){uint16_t n=r.u2();o<<" entries="<<n<<"\n";for(unsigned i=0;i<n;i++){uint16_t st=r.u2(),len=r.u2(),ni=r.u2(),di=r.u2(),slot=r.u2();o<<p<<"  pc="<<st<<".."<<st+len<<" #"<<ni<<" "<<cpDescription(ni)<<" : #"<<di<<" "<<cpDescription(di)<<" slot="<<slot<<"\n";}checkEnd(r,a.name);continue;}
        else if(a.name=="Synthetic"||a.name=="Deprecated"){checkEnd(r,a.name);o<<"\n";continue;}
        else if(a.name=="EnclosingMethod"){uint16_t c=r.u2(),m=r.u2();o<<" class #"<<c<<" // "<<cpDescription(c)<<" method #"<<m;if(m)o<<" // "<<cpDescription(m);}
        else if(a.name=="InnerClasses"){uint16_t n=r.u2();o<<" count="<<n<<"\n";for(unsigned i=0;i<n;i++){uint16_t in=r.u2(),out=r.u2(),nm=r.u2(),fl=r.u2();o<<p<<"  inner=#"<<in<<" // "<<cpDescription(in)<<" outer=#"<<out;if(out)o<<" // "<<cpDescription(out);o<<" name=#"<<nm;if(nm)o<<" // "<<cpDescription(nm);o<<" flags="<<hex(fl,4)<<"\n";}checkEnd(r,a.name);continue;}
        else if(a.name=="MethodParameters"){uint8_t n=r.u1();o<<" count="<<unsigned(n)<<"\n";for(unsigned i=0;i<n;i++){uint16_t ni=r.u2(),fl=r.u2();o<<p<<"  name=#"<<ni;if(ni)o<<" // "<<cpDescription(ni);o<<" flags="<<hex(fl,4)<<"\n";}checkEnd(r,a.name);continue;}
        else if(a.name=="BootstrapMethods"){uint16_t n=r.u2();o<<" count="<<n<<"\n";for(unsigned i=0;i<n;i++){uint16_t ref=r.u2(),argc=r.u2();o<<p<<"  ["<<i<<"] method_ref=#"<<ref<<" // "<<cpDescription(ref)<<" args="<<argc;for(unsigned j=0;j<argc;j++){uint16_t x=r.u2();o<<" #"<<x<<"="<<cpDescription(x);}o<<"\n";}checkEnd(r,a.name);continue;}
        else if(a.name=="StackMapTable"){uint16_t n=r.u2();o<<" frames="<<n<<"\n";int32_t offset=-1;for(unsigned i=0;i<n;i++){uint8_t ft=r.u1();uint16_t delta=0;o<<p<<"  frame_type="<<unsigned(ft);
            if(ft<=63)delta=ft;else if(ft<=127){delta=ft-64;o<<" stack=";verificationType(r,o,*this);}else if(ft==247){delta=r.u2();o<<" stack=";verificationType(r,o,*this);}else if(ft>=248&&ft<=250)delta=r.u2();else if(ft==251)delta=r.u2();else if(ft>=252&&ft<=254){delta=r.u2();o<<" locals=";for(unsigned j=0;j<static_cast<unsigned>(ft-251);j++){if(j)o<<",";verificationType(r,o,*this);}}else if(ft==255){delta=r.u2();uint16_t nl=r.u2();o<<" locals="<<nl<<"{";for(unsigned j=0;j<nl;j++){if(j)o<<",";verificationType(r,o,*this);}uint16_t ns=r.u2();o<<"} stack="<<ns<<"{";for(unsigned j=0;j<ns;j++){if(j)o<<",";verificationType(r,o,*this);}o<<"}";}else throw std::runtime_error("frame_type inválido");offset+=delta+1;o<<" offset="<<offset<<"\n";}checkEnd(r,a.name);continue;}
        else if(a.name=="RuntimeVisibleAnnotations"||a.name=="RuntimeInvisibleAnnotations"){annotations(r,o,*this,depth);checkEnd(r,a.name);o<<"\n";continue;}
        else if(a.name=="RuntimeVisibleParameterAnnotations"||a.name=="RuntimeInvisibleParameterAnnotations"){uint8_t np=r.u1();o<<" parameters="<<unsigned(np)<<"\n";for(unsigned i=0;i<np;i++){o<<p<<"  parameter["<<i<<"]";annotations(r,o,*this,depth+2);o<<"\n";}checkEnd(r,a.name);continue;}
        else if(a.name=="RuntimeVisibleTypeAnnotations"||a.name=="RuntimeInvisibleTypeAnnotations"){uint16_t n=r.u2();o<<" count="<<n<<"\n";for(unsigned i=0;i<n;i++){o<<p<<"  ";typeAnnotation(r,o,*this,depth+1);o<<"\n";}checkEnd(r,a.name);continue;}
        else if(a.name=="AnnotationDefault"){o<<" value=";elementValue(r,o,*this,depth);checkEnd(r,a.name);o<<"\n";continue;}
        else if(a.name=="SourceDebugExtension"){o<<" bytes="<<a.info.size()<<" data=";for(size_t i=0;i<a.info.size();i++)o<<hex(a.info[i],2)<<' ';o<<"\n";continue;}
        else{o<<" (desconhecido, preservado sem interpretação)\n";continue;} checkEnd(r,a.name);o<<"\n";
    }
}
void ClassFile::printCode(std::ostream&o,const Attribute&a,unsigned depth)const{
    Reader r(a.info);uint16_t ms=r.u2(),ml=r.u2();uint32_t len=r.u4();if(len>r.remaining())throw std::runtime_error("code_length inválido");std::vector<uint8_t> code=r.bytes(len);uint16_t ne=r.u2();std::string p(depth,' ');o<<p<<"max_stack="<<ms<<" max_locals="<<ml<<" code_length="<<len<<"\n";disassemble(o,code,*this,depth+2);o<<p<<"exception_table="<<ne<<"\n";for(unsigned i=0;i<ne;i++){uint16_t s=r.u2(),e=r.u2(),h=r.u2(),c=r.u2();o<<p<<"  ["<<s<<","<<e<<") -> "<<h<<" catch #"<<c<<" // "<<(c?cpDescription(c):"all")<<"\n";}uint16_t na=r.u2();std::vector<Attribute> nested;for(unsigned i=0;i<na;i++){Attribute x;x.nameIndex=r.u2();uint32_t n=r.u4();if(n>r.remaining())throw std::runtime_error("atributo aninhado excede Code");x.name=utf8(x.nameIndex);x.info=r.bytes(n);nested.push_back(x);}if(r.remaining())throw std::runtime_error("comprimento de Code inconsistente");printAttributes(o,nested,depth);
}
void ClassFile::printMember(std::ostream&o,const Member&m,bool field)const{uint16_t ni=m.nameIndex,di=m.descriptorIndex;o<<"  "<<utf8(ni)<<" : "<<utf8(di)<<" flags="<<hex(m.flags,4)<<" ("<<access(m.flags,field?1:2)<<") name_index=#"<<ni<<" descriptor_index=#"<<di<<"\n";printAttributes(o,m.attributes,4);}
void ClassFile::display(std::ostream&o)const{const std::string parent=super_?cpDescription(super_):(cpDescription(this_)=="java/lang/Object"?"java/lang/Object (super_class=0)":"<none; zero is reserved for java/lang/Object>");o<<"ClassFile\nmagic: 0xCAFEBABE\nversion: "<<major_<<'.'<<minor_<<" (major="<<major_<<", minor="<<minor_<<")\nconstant_pool_count: "<<cp_.size()<<"\nConstant pool:\n";printPool(o);o<<"access_flags: "<<hex(flags_,4)<<" ("<<access(flags_,0)<<")\nthis_class: #"<<this_<<" // "<<cpDescription(this_)<<"\nsuper_class: #"<<super_<<" // "<<parent<<"\ninterfaces_count: "<<interfaces_.size()<<"\n";for(auto i:interfaces_)o<<"  #"<<i<<" // "<<cpDescription(i)<<"\n";o<<"fields_count: "<<fields_.size()<<"\n";for(const auto&m:fields_)printMember(o,m,true);o<<"methods_count: "<<methods_.size()<<"\n";for(const auto&m:methods_)printMember(o,m,false);o<<"attributes_count: "<<attributes_.size()<<"\n";printAttributes(o,attributes_,2);}
} // namespace jvm
