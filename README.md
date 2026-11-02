# Leitor de arquivos `.class` — Entrega 1

Leitor/exibidor de arquivos `.class` implementado em C++11. A ferramenta interpreta a estrutura binária e exibe a constant pool, flags, hierarquia, interfaces, fields, methods, atributos e bytecodes. Não executa as instruções; `--interpreter` é reservado para uma etapa posterior.

## Compilação no macOS

O `g++` fornecido pelo Xcode/Command Line Tools é Apple Clang. Para compilar com GNU GCC, como pede a disciplina:

```sh
brew install gcc
brew install openjdk@17
export PATH="$(brew --prefix openjdk@17)/bin:$PATH"
GXX="$(ls "$(brew --prefix gcc)"/bin/g++-* | tail -n 1)"
make clean
make CXX="$GXX"
make test CXX="$GXX"
make sanitize CXX="$GXX"
make test CXX="$GXX"
```

O nome do executável GCC acompanha a versão instalada pelo Homebrew; o comando acima seleciona o executável disponível. O script de testes também procura OpenJDK 17 nos prefixos usuais do Homebrew.

## Compilação com GNU GCC em Linux (Debian/Ubuntu)

```sh
sudo apt update
sudo apt install build-essential make openjdk-17-jdk python3
make clean
make CXX=g++
make test CXX=g++
make sanitize CXX=g++
make test CXX=g++
```

Requer C++11 e Python 3 para gerar fixtures binárias de teste. Os testes Java usam `javac`, `java` e `javap` do JDK. OpenJDK 17 compila as fixtures com `--release 8`, produzindo major version 52.

## Opções de compilação

Flags definidas no `Makefile`: `-std=c++11 -Wall -Wextra -Wpedantic -O2 -Iinclude`. Compilador: GNU G++ (`make CXX=g++-16` no macOS com Homebrew; `make` sem `CXX` usa o `g++` do sistema, que no macOS é Apple Clang). Comando equivalente:

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -O2 -Iinclude src/main.cpp src/classfile.cpp -o jvm
```

## Requisitos da entrega

Envio único por grupo no Aprender, até 04/11/2026 às 10h:

- Código-fonte em C/C++ compilável com GCC e as opções de compilação acima.
- Vídeo de 5 minutos: o que foi e o que não foi implementado, como compilar e executar.
- Slides com prints reais das estruturas exibidas pelo leitor.
- Resumo das atividades de cada integrante (`docs/ATIVIDADES_INDIVIDUAIS.md`).

Critérios de avaliação do leitor/exibidor: 70% exibição das estruturas lidas do `.class`; 30% exibição dos mnemônicos dos bytecodes.

## Execução

```sh
./jvm --viewer build/test-classes/Cases.class
./jvm --viewer build/test-classes/AnnotationsFixture.class
```

O segundo exemplo fica disponível após `make test`. Um arquivo inválido ou truncado resulta em mensagem no stderr e código de saída diferente de zero.

## Organização

- `src/main.cpp`: CLI e tratamento de erros.
- `src/classfile.cpp`, `include/classfile.hpp`: leitor, modelos, resolução da constant pool, atributos, exibição e disassembler.
- `tests/java`: classes Java com operações, herança, interface, arrays, exceção, annotations, lambda e classes internas.
- `tests/scripts`: testes automatizados e geradores de fixtures de formato controlado.
- `docs/VERIFICACAO_REQUISITOS.md`: checklist e limitações conferidas.
- `docs/COMPARACAO_JAVAP.md`: comparação feita nas classes Java 8 compiladas.
- `docs/EXEMPLO_REAL_EXECUCAO.txt`: saída capturada do programa lendo uma classe gerada por `javac --release 8`.
- `docs/APRESENTACAO_ENTREGA_1.md`, `docs/SLIDES_ENTREGA_1.md`, `docs/ATIVIDADES_INDIVIDUAIS.md`: materiais da apresentação e do grupo.

## Implementado

- Leitura big-endian de `u1`, `u2`, `u4` e `u8`; validação de `CAFEBABE`, versões, limites e truncamento.
- Tags de constant pool Java 8, slots duplos para `Long`/`Double`, Modified UTF-8 e resolução textual das referências, com índices e valores.
- ClassFile, interfaces, fields, methods, flags e índices de nomes/descritores; métodos sem `Code` são aceitos.
- Atributos Java 8 listados no enunciado: estruturas de Code e tabelas de exceção; stack map frames e verification types; BootstrapMethods; InnerClasses; EnclosingMethod; tabelas de depuração; annotations visíveis/invisíveis, de parâmetros e de tipos; AnnotationDefault; MethodParameters e demais atributos previstos. Atributos desconhecidos permanecem delimitados por `attribute_length` e são informados como não interpretados.
- Mnemônicos dos 202 opcodes atribuídos na JVM Java 8, operandos, branches relativos, operandos da constant pool, `tableswitch`, `lookupswitch` e `wide`.
- `make test` verifica entradas inválidas/truncadas, fixtures controladas, versões de formato 46/50/52 e classes Java 8 reais; compara pontos de referência com `javap -verbose`.

## Limitações verificadas

- Os major versions 46 (Java 2) e 50 (Java 6) foram testados com fixtures estruturais mínimas geradas pelo repositório. Classes reais compiladas para Java 2 e Java 6 ainda não foram testadas; JDK moderno não oferece esses targets.
- `javap` foi usado para comparar versão, atributos, referências e mnemônicos nas fixtures Java 8. Não foi feita comparação com jclasslib.
- O disassembler reconhece os operandos e os imprime, mas não é um verificador JVM completo: não valida todas as restrições semânticas, compatibilidade do tipo CP exigido por cada opcode nem todos os destinos de branch.
- Para atributos conhecidos, os dados são interpretados e apresentados; a validação semântica completa de cada índice/flag do arquivo também está fora do escopo atual.
- O plano de ensino e `5-formato-class-red.pdf` não foram encontrados em `docs/` nem na pasta de documentos pesquisada. O material disponível `../ladeira_aulas_provas/Java-JVM/formato-class.pdf` e a [JVMS Java SE 8 oficial](https://docs.oracle.com/javase/specs/jvms/se8/html/) foram consultados. Os critérios específicos do plano ainda precisam ser confrontados quando o arquivo estiver disponível.

## Ambiente de validação usado

Na sessão de validação: GNU GCC 16.2.0 (Homebrew), OpenJDK 17.0.21.0.3 e Python 3.15.2. `make`, `make test` e a suíte com AddressSanitizer/UBSan passaram; `javac --release 8` e `javap -verbose` foram executados sem warnings da fixture.
