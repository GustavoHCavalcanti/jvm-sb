# Slides — Entrega 1 (6 slides)

## Slide 1 — Problema e objetivo

- Ler e exibir a estrutura de arquivos Java `.class` em C++11.
- Esta etapa não executa bytecodes.
- **Print real:** cabeçalho e versão da `AnnotationsFixture.class`.

## Slide 2 — Estrutura do leitor

- Fluxo: arquivo binário → ClassFile/pool → membros/atributos → disassembler.
- Compilação C++11 com GNU GCC.
- **Print real:** `make` concluído sem warnings; mostrar os arquivos `main.cpp` e `classfile.cpp`.

## Slide 3 — Constant pool e referências

- Mostrar índice, tipo e valor resolvido: `Methodref`, `Class`, `NameAndType`, strings e constantes numéricas.
- **Print real:** primeiras linhas de [`EXEMPLO_REAL_EXECUCAO.txt`](EXEMPLO_REAL_EXECUCAO.txt).

## Slide 4 — Fields, methods e atributos

- Flags, descritores, `Code`, exceções, variáveis locais, StackMapTable, BootstrapMethods, InnerClasses e annotations.
- **Print real:** saída da `AnnotationsFixture.class`; citar a classe `Mark.class` para AnnotationDefault.

## Slide 5 — Disassembler e testes

- Offsets, mnemônicos, operandos, branches, referências CP, switches e wide.
- Testes com classes Java 8 reais e comparação de marcadores com `javap -verbose`.
- **Print real:** saída do `make test` e trecho do método `Cases.compute`.

## Slide 6 — Execução, estado e próximos passos

- Comandos: `make CXX="$GXX"`, `make test CXX="$GXX"`, `./jvm --viewer arquivo.class`.
- Implementado: leitura, resolução, atributos Java 8 e disassembly.
- Pendente: verificação semântica completa, GUI jclasslib-like, interpretador futuro e avaliação dos critérios do plano de ensino ausente.
- **Print real:** erro para arquivo truncado.
