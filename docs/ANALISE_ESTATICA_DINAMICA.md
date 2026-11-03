# Análise estática e dinâmica

Pré-requisitos, instalados manualmente:

```sh
brew install cppcheck
brew install llvm
```

No Linux (Debian/Ubuntu): `sudo apt install cppcheck clang`.

O `llvm` é necessário no macOS porque o Apple Clang 21 falhou no link com `-fsanitize=fuzzer,address`. Nos comandos abaixo, `FUZZ_CXX` aponta para o Clang do Homebrew. No Linux, omita `FUZZ_CXX`.

## A. Análise estática — Cppcheck

**Objetivo:** encontrar possíveis defeitos sem executar o programa.

**Ferramenta:** Cppcheck, com padrão C++11 e as categorias `warning`, `performance` e `portability`. A categoria `style` fica de fora para reduzir mensagens cosméticas.

**Execução:**

```sh
make cppcheck
```

Comando executado pelo alvo:

```sh
cppcheck --language=c++ --std=c++11 --enable=warning,performance,portability -Iinclude src tests/fuzz
```

**Como interpretar:** cada mensagem indica arquivo, linha, severidade e identificador entre colchetes.

| Severidade | Significado | Ação |
|---|---|---|
| `error` | Provável defeito | Analisar e corrigir, se confirmado |
| `warning` | Código possivelmente incorreto | Analisar caso a caso |
| `performance`, `portability` | Risco de desempenho ou de portabilidade | Avaliar a relevância |

Cada aviso pode ser um falso positivo, por isso precisa ser confirmado no código. O alvo termina com sucesso mesmo quando há avisos: leia a saída.

**Como registrar no relatório:** salve a saída e copie para o relatório a versão, o comando e cada aviso, com a decisão tomada.

```sh
mkdir -p build
cppcheck --version
make cppcheck 2>&1 | tee build/cppcheck.txt
```

## B. Análise dinâmica — LibFuzzer

**Objetivo:** executar o parser com entradas geradas automaticamente e detectar crashes e erros de memória.

**Ferramenta:** LLVM LibFuzzer com AddressSanitizer. O alvo `tests/fuzz/fuzz_classfile.cpp` reutiliza `jvm::ClassFile` e chama `display()`, sem duplicar o parser.

**Como compilar:**

```sh
make fuzz-build FUZZ_CXX="$(brew --prefix llvm)/bin/clang++"
```

**Como executar uma sessão curta (cerca de 10 s):**

```sh
make fuzz-smoke FUZZ_CXX="$(brew --prefix llvm)/bin/clang++"
```

O alvo gera sementes em `build/fuzz-corpus` com os scripts Python de `tests/scripts` e executa:

```sh
build/fuzz_classfile -max_total_time=10 -timeout=5 -max_len=65536 -artifact_prefix=build/fuzz-artifacts/ build/fuzz-corpus
```

**Como interpretar:**

- **Sem defeito na sessão:** a saída termina com `Done <N> runs in <T> second(s)` e o `make` termina sem erro.
- **Progresso:** `cov` é a cobertura, `corp` é o tamanho do corpus e `exec/s` é a velocidade.
- **Defeito encontrado:** aparecem `ERROR: AddressSanitizer` ou `ERROR: libFuzzer`, além de `SUMMARY`, e um arquivo `crash-*`, `timeout-*` ou `oom-*` em `build/fuzz-artifacts/`.
- **Exceções:** a rejeição de arquivo inválido por exceção é esperada e não é defeito.
- **Limite da conclusão:** uma sessão sem falhas indica apenas que nada foi encontrado naquele tempo, não que o programa está livre de defeitos.

Para reproduzir um defeito:

```sh
build/fuzz_classfile build/fuzz-artifacts/<arquivo>
```

**Como registrar no relatório:** salve a saída e copie os campos da seção D. Copie também os artefatos que quiser preservar, pois `make clean` apaga `build/`.

```sh
make fuzz-smoke FUZZ_CXX="$(brew --prefix llvm)/bin/clang++" 2>&1 | tee build/fuzz-smoke.txt
```

**Como encerrar:** a sessão termina sozinha após cerca de 10 s, ou no primeiro defeito. Para interromper antes, use `Ctrl+C`.

## C. Demonstração em vídeo

Rode a sequência abaixo em um único terminal:

```sh
make clean
make CXX=g++-16
make test CXX=g++-16
./jvm --viewer build/test-classes/Cases.class
cppcheck --version
make cppcheck
make fuzz-build FUZZ_CXX="$(brew --prefix llvm)/bin/clang++"
make fuzz-smoke FUZZ_CXX="$(brew --prefix llvm)/bin/clang++"
```

O que mostrar na tela:

| Etapa | Evidência |
|---|---|
| Compilação | Linhas do `g++-16` com `-std=c++11`, sem warnings |
| Testes | Todas as linhas `PASS` |
| Leitor | Constant pool com índices e valores; métodos com mnemônicos |
| Cppcheck | Versão, linhas `Checking ...` e os avisos, ou a ausência deles |
| LibFuzzer, compilação | Linha com `-fsanitize=fuzzer,address` |
| LibFuzzer, execução | Linhas `INITED`, `cov`, `exec/s` e a linha final `Done`, ou o defeito encontrado |

## D. Relatório acadêmico

Preencha os campos somente com resultados obtidos em execuções reais. Se um item não foi executado, escreva "não executado".

### Metodologia — Cppcheck

> A análise estática foi feita com o Cppcheck sobre os fontes C++ do leitor e do fuzz target, com padrão C++11 e as categorias `warning`, `performance` e `portability`. Cada aviso foi conferido no código e classificado como defeito ou falso positivo.

| Campo | Resultado |
|---|---|
| Versão do Cppcheck | |
| Sistema e data | |
| Comando | `make cppcheck` |
| Total de avisos | |
| Avisos por severidade | |
| Defeitos confirmados | |
| Falsos positivos | |
| Correções realizadas | |

### Metodologia — LibFuzzer

> A análise dinâmica foi feita com o LLVM LibFuzzer e o AddressSanitizer. O fuzz target grava cada entrada em um arquivo temporário e a processa com o leitor. A sessão curta partiu de sementes geradas pelos scripts de teste do projeto. Por ser curta, ela não comprova a ausência de defeitos.

| Campo | Resultado |
|---|---|
| Versão do Clang | |
| Sistema e data | |
| Comando | `make fuzz-smoke` |
| Duração | |
| Execuções (`Done <N> runs`) | |
| Cobertura final (`cov`) | |
| Tamanho final do corpus (`corp`) | |
| Defeitos encontrados | |
| Artefatos gerados | |
| Causa e correção | |
