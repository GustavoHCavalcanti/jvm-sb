# Leitor de arquivos `.class` — Entrega 1

Leitor/exibidor de arquivos `.class` implementado em C++11. A ferramenta interpreta a estrutura binária e exibe a constant pool, flags, hierarquia, interfaces, fields, methods, atributos e bytecodes. Não executa as instruções; `--interpreter` é reservado para uma etapa posterior.

## Resumo rápido

| Sistema | Instalar | Compilar (com make) | Compilar (sem make) | Executar |
|---|---|---|---|---|
| Windows 10/11 (terminal MSYS2 UCRT64) | MSYS2 + `pacman -S --needed mingw-w64-ucrt-x86_64-gcc make` | `make` | `g++ -std=c++11 -Wall -Wextra -Wpedantic -O2 -Iinclude src/main.cpp src/classfile.cpp -static -o jvm.exe` | `./jvm.exe --viewer arquivo.class` |
| Linux (Debian/Ubuntu) | `sudo apt install build-essential` | `make` | `g++ -std=c++11 -Wall -Wextra -Wpedantic -O2 -Iinclude src/main.cpp src/classfile.cpp -o jvm` | `./jvm --viewer arquivo.class` |
| macOS | `brew install gcc` | `make CXX=g++-16` | `g++-16 -std=c++11 -Wall -Wextra -Wpedantic -O2 -Iinclude src/main.cpp src/classfile.cpp -o jvm` | `./jvm --viewer arquivo.class` |

Para ter um `.class` de exemplo sem JDK: `make exemplo` gera `build/Exemplo.class`. Os testes (`make test`) precisam de Python 3 e JDK 17. Versão em texto: [INSTRUCOES_COMPILACAO.txt](INSTRUCOES_COMPILACAO.txt). O workflow [.github/workflows/build.yml](.github/workflows/build.yml) compila e testa nos três sistemas a cada push.

## Compilação e execução no Windows 10/11

> **Estado da validação:** a validação em Windows é feita pelo GitHub Actions (`windows-latest`, MSYS2 UCRT64); veja [Ambiente de validação usado](#ambiente-de-validação-usado).

Estratégia: **MSYS2 UCRT64**, que fornece GCC (MinGW-w64) e GNU Make. O leitor usa apenas a biblioteca padrão de C++ e a API do Windows; não depende de Java, Python, Cppcheck ou LLVM para compilar e executar.

### Ferramentas necessárias

| Para | Ferramenta | Obrigatória? |
|---|---|---|
| Compilar e executar o leitor | MSYS2 UCRT64, GCC e GNU Make | Sim |
| Executar `make test` | Python 3 | Só para os testes |
| Executar o teste com classes Java reais | JDK 17 | Opcional: sem ele, esse teste aparece como `SKIP` |

### 1. Instalar o MSYS2

Baixe o instalador em <https://www.msys2.org> e use a pasta padrão `C:\msys64`. Se preferir, instale pelo PowerShell:

```powershell
winget install --id MSYS2.MSYS2 -e
```

### 2. Abrir o terminal certo

No menu Iniciar, abra **MSYS2 UCRT64**. O prompt deve mostrar `UCRT64` em destaque. Os próximos comandos são digitados nesse terminal, exceto os marcados como PowerShell.

### 3. Instalar GCC e Make

```sh
pacman -Syu
```

Se o terminal fechar ao final da atualização, abra o **MSYS2 UCRT64** de novo. Depois:

```sh
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-gcc make
```

O pacote `make` fornece o comando `make`. O pacote `mingw-w64-ucrt-x86_64-make` não serve para estes comandos, pois instala apenas `mingw32-make`.

### 4. Verificar a instalação

```sh
which g++            # esperado: /ucrt64/bin/g++
g++ -dumpmachine     # esperado: x86_64-w64-mingw32
g++ --version
make --version
```

### 5. Compilar

Entre na pasta do projeto. O disco `C:\` aparece como `/c`, e caminhos com espaços ficam entre aspas:

```sh
cd "/c/Users/SEU_USUARIO/Desktop/jvm-sb"
make clean
make
```

O resultado é `jvm.exe`, com as bibliotecas do GCC ligadas estaticamente. Ele também executa fora do terminal UCRT64.

### 6. Executar com um `.class`

`make exemplo` gera `build/Exemplo.class` a partir de `examples/gera_exemplo.cpp`, usando apenas o compilador:

```sh
make exemplo
./jvm.exe --viewer build/Exemplo.class
```

A saída mostra Constant Pool, campos, métodos, atributos e bytecodes. Para outro arquivo, informe o caminho dele:

```sh
./jvm.exe --viewer "/c/Users/SEU_USUARIO/Meus Exemplos/Teste.class"
./jvm.exe --viewer build/Exemplo.class | less
./jvm.exe --viewer build/Exemplo.class > saida.txt
```

### 7. Executar os testes

Os testes usam Bash, `grep` e `head`, já incluídos no MSYS2, e Python para gerar as fixtures binárias:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-python
make test
```

O sétimo teste compila classes reais e precisa de um JDK. Para incluí-lo, instale o JDK pelo **PowerShell**, reabra o terminal UCRT64 e rode `make test` de novo:

```powershell
winget install --id EclipseAdoptium.Temurin.17.JDK -e
```

O `run.sh` procura o JDK em `C:\Program Files\Eclipse Adoptium`, `C:\Program Files\Java` e `C:\Program Files\Microsoft`. Resultado esperado: sete linhas `PASS`, ou seis linhas `PASS` e uma `SKIP` sem JDK.

`make sanitize`, `make fuzz-build` e `make fuzz-smoke` valem apenas para macOS e Linux: o GCC do UCRT64 não traz AddressSanitizer, e o alvo do LibFuzzer usa `unistd.h` e `/tmp`. Nenhum deles é necessário para compilar ou executar o leitor.

### PowerShell ou MSYS2 UCRT64?

| | MSYS2 UCRT64 (recomendado) | PowerShell |
|---|---|---|
| `g++` e `make` | Disponíveis após a seção 3 | Só com `C:\msys64\ucrt64\bin` no PATH; `make` não fica disponível |
| Caminhos | `/c/Users/Nome/arquivo.class` | `C:\Users\Nome\arquivo.class` |
| Executar | `./jvm.exe --viewer arquivo.class` | `.\jvm.exe --viewer arquivo.class` |
| `make test` | Funciona | Não funciona, pois depende de Bash |
| Salvar a saída com `>` | Preserva o UTF-8 | O Windows PowerShell 5.1 pode alterar a codificação |

Depois de compilado, `jvm.exe` executa nos dois. Para compilar sem Make no PowerShell:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
g++ -std=c++11 -Wall -Wextra -Wpedantic -O2 -Iinclude src/main.cpp src/classfile.cpp -static -o jvm.exe
```

### Solução de problemas

| Mensagem | Causa | Solução |
|---|---|---|
| `g++ não é reconhecido...` (PowerShell ou CMD) | O terminal não conhece o GCC do MSYS2 | Use o terminal **MSYS2 UCRT64**. Para continuar no PowerShell, rode a linha `$env:Path` acima |
| `g++: command not found` | O GCC não foi instalado ou o terminal aberto é outro, como `MSYS` ou `MINGW64` | Abra o **MSYS2 UCRT64** e repita a seção 3 |
| `make: command not found` | Falta o pacote `make` | `pacman -S --needed make` |
| `file format not recognized` ao ligar | Há arquivos `.o` compilados em outro sistema | `make clean` e `make` |
| `FAIL: ./jvm não executou` | Há um executável antigo ou de outro sistema | `make clean` e `make` |
| `FAIL: os testes precisam de Python 3` | Falta o Python | Veja a seção 7 |
| Acentos incorretos no console | Fonte ou terminal sem suporte a UTF-8 | Use o terminal UCRT64 ou o Windows Terminal |

## Compilação e execução no macOS

O `g++` fornecido pelo Xcode/Command Line Tools é Apple Clang. Para compilar com GNU GCC, como pede a disciplina:

```sh
brew install gcc
brew install openjdk@17
export PATH="$(brew --prefix openjdk@17)/bin:$PATH"
GXX="$(ls "$(brew --prefix gcc)"/bin/g++-* | tail -n 1)"
make clean
make CXX="$GXX"
make exemplo CXX="$GXX"
./jvm --viewer build/Exemplo.class
make test CXX="$GXX"
make sanitize CXX="$GXX"
make test CXX="$GXX"
```

O nome do executável GCC acompanha a versão instalada pelo Homebrew; o comando acima seleciona o executável disponível. O script de testes também procura OpenJDK 17 nos prefixos usuais do Homebrew. O JDK só é necessário para o teste com classes Java reais.

## Compilação e execução no Linux (Debian/Ubuntu)

```sh
sudo apt update
sudo apt install build-essential make openjdk-17-jdk python3
make clean
make CXX=g++
make exemplo CXX=g++
./jvm --viewer build/Exemplo.class
make test CXX=g++
make sanitize CXX=g++
make test CXX=g++
```

Para compilar e executar o leitor, bastam `build-essential` e `make`. Os testes usam Python 3 para gerar fixtures binárias e `javac`, `java` e `javap` do JDK. OpenJDK 17 compila as fixtures com `--release 8`, produzindo major version 52.

## Opções de compilação

Flags definidas no `Makefile`: `-std=c++11 -Wall -Wextra -Wpedantic -O2 -Iinclude`. Com MinGW-w64, o `Makefile` acrescenta `-static` na ligação e gera `jvm.exe`. Compilador: GNU G++ (`make CXX=g++-16` no macOS com Homebrew; `make` sem `CXX` usa o `g++` do sistema, que no macOS é Apple Clang). Comando equivalente:

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
./jvm --viewer build/Exemplo.class
./jvm --viewer build/test-classes/Cases.class
./jvm --viewer build/test-classes/AnnotationsFixture.class
```

No Windows, use `./jvm.exe`. O primeiro exemplo fica disponível após `make exemplo`; os outros dois, após `make test` com JDK. Um arquivo inválido ou truncado resulta em mensagem no stderr e código de saída diferente de zero.

## Organização

- `src/main.cpp`: CLI e tratamento de erros.
- `src/classfile.cpp`, `include/classfile.hpp`: leitor, modelos, resolução da constant pool, atributos, exibição e disassembler.
- `examples/gera_exemplo.cpp`: gerador do `.class` de demonstração, sem dependências além do compilador.
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

Windows 10/11 e Linux: a validação é feita pelo workflow `.github/workflows/build.yml`. Em `ubuntu-latest`, `macos-latest` e `windows-latest` (MSYS2 UCRT64), ele roda `make`, `make test` (7 PASS), `make exemplo`, `make demo` e a compilação sem make. No Windows, também executa `jvm.exe` no PowerShell, fora do MSYS2. Confira o resultado mais recente na aba **Actions** do repositório.
