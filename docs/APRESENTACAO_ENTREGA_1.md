# Roteiro de apresentação — Entrega 1 (até 5 minutos)

## Roteiro cronometrado (4 min 50 s)

1. **Objetivo (15 s):** leitor/exibidor de `.class` em C++11 como primeira etapa da JVM. O programa analisa as instruções, sem executá-las.
2. **Arquitetura (25 s):** mostrar `main.cpp`, `classfile.hpp` e `classfile.cpp`; leitura binária com vetores/string e controle de memória por RAII.
3. **ClassFile (30 s):** executar o leitor e mostrar magic, versão, `this_class`, superclasse e interfaces.
4. **Constant pool (40 s):** mostrar entradas `Methodref`, `Class`, `NameAndType`, `Long`/`Double`, com índices e valores resolvidos.
5. **Fields, methods e atributos (35 s):** mostrar flags, descritores, `Code`, exceptions, tabelas de debug, annotations e frames.
6. **Bytecodes (45 s):** exibir offsets, mnemônicos, operandos e referências CP; mostrar `switch`, `wide`, chamada virtual e invokedynamic.
7. **Demonstração terminal (35 s):** compilar com GNU GCC, executar a classe e rodar `make test`.
8. **Estado e limites (50 s):** apresentar testes e sanitizers; dizer que Java 8 real foi verificado e Java 2/6 apenas com fixtures estruturais. O disassembler não faz verificação semântica completa, não existe interface gráfica nem modo interpretador. O plano de ensino não estava disponível para auditoria.
9. **Próximos passos (15 s):** concluir verificação com o plano da disciplina e avançar para o interpretador na próxima etapa.

## Comandos para a gravação

```sh
GXX="$(ls "$(brew --prefix gcc)"/bin/g++-* | tail -n 1)"
export PATH="$(brew --prefix openjdk@17)/bin:$PATH"
make clean
make CXX="$GXX"
make test CXX="$GXX"
./jvm --viewer build/test-classes/AnnotationsFixture.class
```

## Capturas reais

- Captura 1: saída de `./jvm --viewer build/test-classes/AnnotationsFixture.class` (major 52, pool, lambda/invokedynamic, atributos e bytecodes).
- Captura 2: trecho de fields/methods e `Code` da classe `Cases.class`.
- Captura 3: saída final de `make test`.
- Captura 4: mensagem de erro para `build/invalid.class` ou `build/Truncated.class`.

O arquivo [`EXEMPLO_REAL_EXECUCAO.txt`](EXEMPLO_REAL_EXECUCAO.txt) contém uma transcrição textual real da captura 1. As imagens dos slides devem ser tiradas pela equipe no terminal durante a gravação.
