#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p build
if ! (java -version >/dev/null 2>&1); then
  # O MSYS2 não herda o PATH do Windows; por isso os JDKs do Windows são procurados aqui.
  for jdk_bin in /opt/homebrew/opt/openjdk@17/bin /usr/local/opt/openjdk@17/bin \
      "/c/Program Files/Eclipse Adoptium/"jdk-*/bin "/c/Program Files/Java/"jdk-*/bin "/c/Program Files/Microsoft/"jdk-*/bin; do
    if [ -x "$jdk_bin/java" ] || [ -x "$jdk_bin/java.exe" ]; then PATH="$jdk_bin:$PATH"; export PATH; break; fi
  done
fi
# Sem argumentos, o leitor mostra o uso e sai com 2; outro código indica executável ausente ou de outro sistema.
jvm_status=0
./jvm >/dev/null 2>&1 || jvm_status=$?
if [ "$jvm_status" -ne 2 ]; then
  echo "FAIL: ./jvm não executou (código $jvm_status). Recompile com: make clean && make"; exit 1
fi
PYTHON=
for candidate in python3 python; do
  if "$candidate" -c 'import sys; sys.exit(sys.version_info < (3,))' >/dev/null 2>&1; then PYTHON=$candidate; break; fi
done
if [ -z "$PYTHON" ]; then
  echo "FAIL: os testes precisam de Python 3 para gerar as fixtures binárias (veja o README)."; exit 1
fi
printf 'bad' > build/invalid.class
if ./jvm --viewer build/invalid.class >/dev/null 2>&1; then
  echo "FAIL: arquivo inválido foi aceito"; exit 1
fi
echo "PASS: rejeição de entrada inválida"
"$PYTHON" tests/scripts/make_minimal.py build/Minimal.class
head -c 12 build/Minimal.class > build/Truncated.class
if ./jvm --viewer build/Truncated.class >/dev/null 2>&1; then
  echo "FAIL: arquivo truncado foi aceito"; exit 1
fi
./jvm --viewer build/Minimal.class > build/Minimal.viewer.txt
grep -q 'this_class: #2 // Minimal' build/Minimal.viewer.txt
grep -q 'methods_count: 1' build/Minimal.viewer.txt
grep -q '0: iconst_1' build/Minimal.viewer.txt
grep -q '1: ireturn' build/Minimal.viewer.txt
echo "PASS: leitura de fixture binária, método e bytecodes"
for version in 46 50 52; do
  "$PYTHON" tests/scripts/make_minimal.py "build/Minimal-${version}.class" "$version"
  ./jvm --viewer "build/Minimal-${version}.class" | grep -q "major=${version}"
done
echo "PASS: leitura estrutural de versões major Java 2, 6 e 8 (46, 50, 52)"
"$PYTHON" tests/scripts/make_minimal.py build/Object.class --root
./jvm --viewer build/Object.class | grep -q 'super_class: #0 // java/lang/Object (super_class=0)'
echo "PASS: caso super_class zero para java/lang/Object"
"$PYTHON" tests/scripts/make_attribute_fixture.py build/Attributes.class
./jvm --viewer build/Attributes.class > build/Attributes.viewer.txt
for attr in StackMapTable BootstrapMethods InnerClasses EnclosingMethod RuntimeVisibleAnnotations RuntimeVisibleTypeAnnotations RuntimeVisibleParameterAnnotations AnnotationDefault MethodParameters SourceDebugExtension; do
  grep -q "$attr" build/Attributes.viewer.txt
done
grep -q 'element #.* = I #.*37' build/Attributes.viewer.txt
echo "PASS: decodificação estrutural de atributos Java 8 e anotações"
"$PYTHON" tests/scripts/make_bytecode_fixture.py build/Bytecodes.class
./jvm --viewer build/Bytecodes.class > build/Bytecodes.viewer.txt
grep -q 'tableswitch default:24 low:-1 high:0 -1:24 0:24' build/Bytecodes.viewer.txt
grep -q 'lookupswitch default:20 7:20' build/Bytecodes.viewer.txt
grep -q 'wide iload 300' build/Bytecodes.viewer.txt
echo "PASS: operands de tableswitch, lookupswitch e wide"
if ! command -v javac >/dev/null 2>&1 || ! command -v java >/dev/null 2>&1 || ! (java -version >/dev/null 2>&1); then
  echo "SKIP: fixture Java 8 requer javac e Java Runtime."
  exit 0
fi
mkdir -p build/test-classes
javac -encoding UTF-8 -g -parameters --release 8 -d build/test-classes tests/java/Cases.java tests/java/AnnotationsFixture.java
./jvm --viewer build/test-classes/Cases.class > build/Cases.viewer.txt
javap -verbose -classpath build/test-classes Cases > build/Cases.javap.txt
grep -q 'CAFEBABE' build/Cases.viewer.txt
grep -q 'Constant pool:' build/Cases.viewer.txt
grep -q 'tableswitch\|lookupswitch' build/Cases.viewer.txt
grep -q 'invokevirtual' build/Cases.viewer.txt
grep -q 'LineNumberTable' build/Cases.viewer.txt
grep -q 'Methodref #.*// java/lang/String.length:()I' build/Cases.viewer.txt
grep -q 'Long 1234567890123' build/Cases.viewer.txt
grep -q 'Long -42' build/Cases.viewer.txt
grep -q 'Double 6.283185307179586' build/Cases.viewer.txt
grep -q 'Float 0.100000001' build/Cases.viewer.txt
# LC_ALL=C compara bytes: o grep do MSYS2 (wchar_t de 16 bits) não casa
# caracteres UTF-8 de 4 bytes, como o emoji, em locale UTF-8.
LC_ALL=C grep -q 'café 😀' build/Cases.viewer.txt
grep -q '\\u0000' build/Cases.viewer.txt
grep -q 'interfaces_count: 1' build/Cases.viewer.txt
grep -q 'newarray 10 // int' build/Cases.viewer.txt
grep -q 'iastore' build/Cases.viewer.txt
grep -q 'athrow' build/Cases.viewer.txt
grep -q 'Exceptions (' build/Cases.viewer.txt
grep -q 'major version: 52' build/Cases.javap.txt
grep -q 'major=52' build/Cases.viewer.txt
grep -q 'tableswitch\|lookupswitch' build/Cases.javap.txt
for classfile in build/test-classes/*.class; do
  ./jvm --viewer "$classfile" > "build/$(basename "$classfile").viewer.txt"
done
javap -verbose -classpath build/test-classes 'AnnotationsFixture$1Local' > build/Local.javap.txt
grep -q 'StackMapTable' build/AnnotationsFixture.class.viewer.txt
grep -q 'BootstrapMethods' build/AnnotationsFixture.class.viewer.txt
grep -q 'RuntimeVisibleTypeAnnotations' build/AnnotationsFixture.class.viewer.txt
grep -q 'InnerClasses' 'build/AnnotationsFixture$Nested.class.viewer.txt'
grep -q 'EnclosingMethod' 'build/AnnotationsFixture$1Local.class.viewer.txt'
grep -q 'AnnotationDefault' build/Mark.class.viewer.txt
grep -q 'major version: 52' build/Local.javap.txt
echo "PASS: fixture Java 8, confronto de versão/opcode com javap, pool, bytecodes e atributos"
