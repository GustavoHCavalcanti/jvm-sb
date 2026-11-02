# Comparação com `javap -verbose`

Comparação realizada com OpenJDK 17.0.21.0.3, compilando as fixtures com `javac --release 8`. O teste automatizado executa:

```sh
javap -verbose -classpath build/test-classes Cases
./jvm --viewer build/test-classes/Cases.class
```

Os resultados conferidos automaticamente são: major version 52, ocorrência de switch, `LineNumberTable`, chamadas `invokevirtual`, constant-pool `Methodref` resolvida e atributos de exceptions. O script também submete todas as classes geradas pelo compilador ao leitor e verifica os atributos de lambda, annotations, classe interna/local e frames.

Na inspeção do `AnnotationsFixture$1Local.class`, o `javap` e o leitor apresentaram os mesmos itens estruturais: superclass, field sintético `this$0`, construtor, `Code`, instruções `aload_0`, `aload_1`, `putfield`, `invokespecial`, `return`, tabelas de linha/variáveis, parâmetros, `SourceFile`, `EnclosingMethod` e `InnerClasses`. A apresentação difere porque `javap` usa o formato de listagem tradicional e o leitor imprime referências no formato próprio do projeto; offsets, índices, descritores e valores foram conferidos.

O script não faz comparação textual linha a linha de toda a saída. As provas automatizadas e as fixtures estão em `tests/scripts/run.sh`; a saída real capturada está em `EXEMPLO_REAL_EXECUCAO.txt`.
