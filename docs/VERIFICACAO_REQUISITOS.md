# Checklist de requisitos — Entrega 1

**Legenda:** Completo = implementado e verificado no escopo indicado; Parcial = implementação existe, mas falta verificação ou validação semântica; Pendente = não concluído.

## Referências da disciplina

| Requisito | Estado | Evidência/observação |
|---|---|---|
| Ler o plano de ensino disponível em `docs/` | **Pendente** | Nenhum PDF está em `docs/`; não localizei `26-2-SB-01-plano_ensino.pdf` na pasta de documentos pesquisada. Os critérios de avaliação do plano não podem ser conferidos ainda. |
| Ler a especificação `.class` do professor | **Completo** | Li `../ladeira_aulas_provas/Java-JVM/formato-class.pdf` (1380 linhas extraídas), que apresenta ClassFile, pool, fields, methods e atributos básicos. |
| Consultar a JVMS Java SE 8 | **Completo** | Consulta da especificação oficial, sobretudo capítulos 4 e 6: [JVMS Java SE 8](https://docs.oracle.com/javase/specs/jvms/se8/html/). |
| Divergências entre material do professor e especificação | **Completo, sem divergência identificada** | O PDF disponível declara ser baseado na JVMS Java SE 8. A comparação cobre os tópicos do leitor; o plano ausente ainda pode conter critérios adicionais. |

## Implementação

| Requisito | Estado | Evidência/limite |
|---|---|---|
| C++11, RAII, GCC, flags `-std=c++11 -Wall -Wextra -Wpedantic` | **Completo** | Compilado sem warnings com GNU GCC 16.2.0 e também com o `g++` Apple Clang disponível. |
| Leitura binária `u1/u2/u4/u8`, big-endian, magic, versão, truncamento e limites | **Completo** | `src/classfile.cpp`; teste de magic inválido e arquivo truncado. Limite de tamanho de entrada: 512 MiB. |
| ClassFile: pool, flags, classe pai, interfaces, fields, methods, atributos | **Completo** | Lidos e exibidos com índices; `super_class=0` é identificado como convenção de `java/lang/Object`; método abstrato/nativo sem `Code` não exige esse atributo. |
| Constant pool Java 8, slots duplos, Modified UTF-8, números | **Completo** | Tags 1–12, 15, 16 e 18; `Long`/`Double` ocupam dois índices; números são impressos com precisão round-trip. A fixture Java testa `NUL`, caracteres acentuados e par surrogate suplementar. |
| Resolução de referências à constant pool | **Completo para referências suportadas** | Índices e valores são exibidos para classes, strings, fields, methods, name/type, handles e invokedynamic. Restrições semânticas completas de cada entrada ainda não são verificadas. |
| Flags de classe, field e método; nome/descritor | **Completo para flags definidos no formato Java 8** | Os bits reservados não são rejeitados/verificados. |
| Atributos Java 8 pedidos | **Completo na decodificação estrutural** | Code, StackMapTable, BootstrapMethods, InnerClasses, EnclosingMethod, Exceptions, ConstantValue, Synthetic, Signature, SourceFile, SourceDebugExtension, tabelas de debug, atributos de annotations, AnnotationDefault, MethodParameters e Deprecated têm campos lidos/exibidos. A validação semântica de todos os valores e flags ainda não equivale ao verificador da JVM. |
| Atributo desconhecido e alinhamento | **Completo** | O conteúdo é delimitado por `attribute_length`, preservado e rotulado como não interpretado. |
| Mnemônicos e operandos dos opcodes Java 8 | **Parcial** | Há 202 nomes de opcode e decodificação de operandos, referências CP, branches, `wide`, `tableswitch` e `lookupswitch`. Não é verificador JVM: não valida toda a semântica dos operandos nem todos os destinos de branch. |
| Apresentação semelhante ao jclasslib | **Parcial** | Exibição textual organizada com resolução; não há interface visual em árvore nem comparação direta com jclasslib. |
| Preparação para interpretador | **Parcial** | Estruturas básicas são reutilizáveis, mas o modo `--interpreter` é reservado e não implementado. |

## Testes executados

| Caso | Estado | Resultado |
|---|---|---|
| Build C++11 com GNU GCC | **Completo** | GCC 16.2.0; sem warnings. |
| Build com sanitizers | **Completo** | AddressSanitizer e UndefinedBehaviorSanitizer, sem erros na suíte. |
| Entrada inválida e truncada | **Completo** | Rejeitadas. |
| Versões major 46/50/52 (Java 2/6/8) | **Parcial** | Fixtures estruturais mínimas; somente major 52 foi também gerado por compilador Java real. |
| Classes Java 8 reais e `javap -verbose` | **Completo para a matriz da fixture** | Compiladas com OpenJDK 17 `--release 8`; parser leu todos os `.class` produzidos. Testes comparam major version, mnemônicos e presença/resolução de pool, métodos e atributos chave com `javap`. |
| Cobertura funcional de casos do enunciado | **Completo para a fixture de integração** | Operações, variáveis, métodos estáticos/de instância, construtor, herança, interface, arrays, strings, loops, condicionais, switch, exceções, long/double, debug, classes abstratas e múltiplos métodos estão em `tests/java/Cases.java`. |
| Annotations, inner/local classes, lambda/invokedynamic, StackMapTable e BootstrapMethods | **Completo para a fixture de integração** | Exercitados por `tests/java/AnnotationsFixture.java` e classes geradas correspondentes. Atributos adicionais são exercitados pela fixture estrutural. |
| Java 2/6 compilado por javac e jclasslib | **Pendente** | JDK 17 atual não gera targets Java 2/6; jclasslib não foi instalado/usado. |

## Documentação e material de entrega

| Requisito | Estado | Evidência/observação |
|---|---|---|
| README com dependências e comandos macOS/GNU | **Completo** | [`README.md`](../README.md). |
| Roteiro de vídeo de até 5 minutos | **Completo** | [`APRESENTACAO_ENTREGA_1.md`](APRESENTACAO_ENTREGA_1.md). |
| Slides 5–7 com capturas reais a inserir | **Completo como roteiro** | [`SLIDES_ENTREGA_1.md`](SLIDES_ENTREGA_1.md); a equipe deve capturar as telas durante a apresentação. |
| Exemplo de saída real | **Completo** | [`EXEMPLO_REAL_EXECUCAO.txt`](EXEMPLO_REAL_EXECUCAO.txt), produzido pelo executável atual lendo classe Java 8 compilada. |
| Registro de contribuições verdadeiras | **Completo como modelo** | [`ATIVIDADES_INDIVIDUAIS.md`](ATIVIDADES_INDIVIDUAIS.md); campos permanecem em branco para o grupo preencher. |
| Organização de arquivos e submissão no Aprender | **Parcial** | Arquivos do projeto estão organizados; confirmar no Aprender o formato/todos os artefatos pedidos pelo plano ausente. |
