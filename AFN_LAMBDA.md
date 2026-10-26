# Módulo de conversão de AFN lambda para AFN

Este documento descreve integralmente a parte implementada em `afn_lambda.c`
e `afn_lambda.h`, sua integração ao menu, as entradas e os testes.

## 1. Objetivo e posição no trabalho

A função deste módulo é receber um autômato finito não determinístico com
transições lambda (AFN-λ) e construir um AFN equivalente sem essas transições.
Equivalente significa reconhecer exatamente a mesma linguagem, inclusive a
palavra vazia quando ela é aceita pelo original.

Uma transição lambda muda o estado sem consumir um símbolo da entrada.
O AFN resultante pode continuar tendo vários destinos para o mesmo símbolo.
Remover lambda não é determinizar nem minimizar.

O fluxo conceitual do trabalho é:

```text
AFN com lambda → AFN sem lambda → AFD → AFD mínimo
     este módulo             módulos dos colegas
```

O programa recebe a descrição do autômato. Não oferece uma opção de testar
palavras no menu de uso.
O terminal exibe primeiro o AFN original, incluindo as transições lambda,
e depois o AFN resultante, sem lambda; não há exportação nem envio automático
para a próxima etapa do grupo.

## 2. Arquivos da implementação

| Arquivo | Responsabilidade |
|---|---|
| `afn_lambda.c` | Estrutura interna, leitura, validação, conversão, impressão e submenu |
| `afn_lambda.h` | Declaração da função pública de entrada do módulo |
| `main.c` | Inclui o cabeçalho e chama o módulo na opção 2 |
| `ex_afn_lambda.txt` | Exemplo válido com três símbolos: x, y e 0 |
| `README.md` | Instruções rápidas para compilar e usar |

`afd.c`, `minimizar_afd.c` e seus cabeçalhos pertencem às outras partes já
existentes do projeto. Este módulo não precisa alterar seus algoritmos.

## 3. Compilação e execução

Na pasta do projeto, com GCC instalado:

```sh
gcc -std=c11 -Wall -Wextra -pedantic -g main.c afd.c commons.c minimizar_afd.c conversor_AFN_para_AFD.c afn_multiplos_estados.c afn_lambda.c -o simulador
./simulador
```

No VS Code, execute os comandos acima no terminal integrado, na pasta do projeto.

No menu principal, escolha `2`. No submenu:

- `1`: cadastrar pelo teclado;
- `2`: ler um arquivo de texto;
- `0`: voltar ao menu principal.

Após a conversão ou um erro de leitura, pressione Enter para retornar.
No menu principal, `0` encerra. Fim de entrada (EOF) também encerra o menu.

O código usa apenas C e a biblioteca padrão.

## 4. Convenções e limites

- Estados numerados de `0` até `n−1`, exibidos como `q0`, `q1`, etc.
- Entre 1 e 100 estados e exatamente um estado inicial.
- Entre 1 e 26 símbolos distintos no alfabeto.
- Cada símbolo é um caractere ASCII visível, de código 33 a 126.
- Letras, dígitos e pontuação são permitidos. Maiúsculas e minúsculas diferem.
- Espaços, acentos, emojis e símbolos com vários caracteres não são aceitos.
- `l` (L minúsculo) é reservado para lambda e não pode integrar o alfabeto.
  A letra `L` maiúscula pode ser um símbolo comum.
- Zero estados finais e zero transições são permitidos.
- Estados finais repetidos são rejeitados.
- Transições idênticas repetidas têm o mesmo efeito que uma única transição.
- Uma transição para vários destinos é informada uma vez por destino.
- O alfabeto vazio não é aceito nesta implementação.

O uso de `l` mantém a convenção da versão inicial do módulo. Ele representa
lambda no arquivo e no teclado; não representa um caractere consumido.

## 5. Entrada pelo teclado

O módulo solicita, nesta ordem:

1. Quantidade de estados.
2. Quantidade de símbolos.
3. Cada símbolo do alfabeto, um por linha.
4. Número do estado inicial.
5. Quantidade de estados finais.
6. Cada estado final, um por linha.
7. Quantidade de transições.
8. Para cada transição: origem, símbolo e destino, cada um em sua própria linha.

Valores inteiros inválidos são solicitados novamente. Símbolos inválidos ou
repetidos também são solicitados novamente. Na leitura interativa de símbolos,
digite apenas o caractere, sem espaços antes ou depois.

## 6. Entrada por arquivo

Digite o nome do arquivo, por exemplo `ex_afn_lambda.txt`, ou seu caminho
completo. Caminhos com espaços são permitidos e devem ser escritos sem aspas.
Um caminho relativo é procurado a partir do diretório em que o programa executa.

O arquivo segue esta estrutura:

```text
quantidade_estados quantidade_simbolos
simbolo_1 simbolo_2 ... simbolo_m
estado_inicial
quantidade_finais final_1 final_2 ...
quantidade_transicoes
origem simbolo destino
origem simbolo destino
...
```

Espaços e quebras de linha separam os campos; a divisão visual acima facilita
a leitura, mas não é obrigatória. A quantidade de transições é obrigatória.
Não inclua comentários nem texto adicional depois da última transição.
Se a quantidade de finais for zero, não escreva números de finais.

Arquivos da versão anterior, com alfabeto fixo a/b, precisam receber a quantidade
de símbolos e seus símbolos imediatamente após a quantidade de estados.
O leitor não detecta automaticamente o formato antigo.

### Exemplo completo

Conteúdo de `ex_afn_lambda.txt`:

```text
3 3
x y 0
0
1 2
4
0 l 1
1 x 1
1 y 2
2 0 2
```

Interpretação: três estados, alfabeto `{x,y,0}`, inicial q0, único final q2,
e quatro transições. q0 chega a q1 por lambda, q1 tem um laço com x e chega
a q2 com y, e q2 tem um laço com o símbolo `0`.

Nesse arquivo, o `0` entre origem e destino é um símbolo. Em outros campos,
o número `0` pode identificar um estado ou indicar uma quantidade.
A posição de cada campo elimina a ambiguidade.

## 7. Representação interna

A estrutura `Automato` é privada ao arquivo C:

```c
typedef struct {
    int n, m, inicial;
    char alfabeto[26];
    unsigned char *finais;
    unsigned char *delta;
} Automato;
```

- `n`: quantidade de estados;
- `m`: quantidade de símbolos;
- `inicial`: índice do estado inicial;
- `alfabeto[s]`: caractere correspondente ao índice s;
- `finais[q]`: 1 se q é final, 0 caso contrário;
- `delta`: matrizes de adjacência armazenadas em um vetor alocado dinamicamente.

Há `m+1` matrizes n×n. As primeiras m correspondem aos símbolos; a última,
no índice m, contém as transições lambda.

A macro `D(a,s,i,j)` acessa a posição:

```text
((s * n) + i) * n + j
```

O valor 1 indica uma transição de i para j no símbolo de índice s; 0 indica
ausência. O AFN de saída mantém uma matriz lambda alocada, porém zerada.
Ela não é exibida e não contém transições.

A matriz auxiliar `fecho` tem n×n posições, acessadas por `C(i,j)`.
O vetor auxiliar `apos_simbolo` tem n posições.

## 8. Algoritmo de conversão

Considere E(q), o conjunto de estados alcançáveis de q usando zero ou mais
transições lambda. Esse conjunto inclui o próprio q.

### 8.1 Calcular os fechos lambda

A matriz de fechos começa com a identidade e as transições lambda diretas:

```text
C(i,j) = (i == j) OU existe transição lambda de i para j
```

O algoritmo de Warshall considera cada estado k como intermediário:

```text
para cada k:
    para cada i que alcança k:
        para cada j alcançável de k:
            marcar que i alcança j
```

Ao final, C representa caminhos lambda de qualquer comprimento, inclusive
ciclos. Não é necessário percorrer recursivamente os ciclos.

### 8.2 Construir as transições sem lambda

Para cada estado q e cada símbolo x:

```text
delta_novo(q,x) = E(mover(E(q),x))
```

Aqui, E de um conjunto é a união dos fechos de seus elementos. A implementação:

1. Examina todos os estados do fecho de q.
2. Reúne os destinos das transições com x em `apos_simbolo`.
3. Inclui o fecho lambda de cada destino na transição resultante.

Assim, uma única transição do resultado representa o caminho original:
zero ou mais lambdas, um símbolo x e zero ou mais lambdas.

### 8.3 Atualizar os finais

Um estado q será final no resultado quando seu fecho contiver ao menos um
estado final original:

```text
F_novo = { q | E(q) intersecta F_original }
```

Isso é essencial para preservar a palavra vazia. Se o inicial alcança um final
apenas por lambda, ele também precisa ser final no AFN resultante.

### 8.4 Preservação da linguagem

Cada transição nova corresponde a um caminho válido do original com exatamente
um símbolo consumido. Inversamente, os trechos lambda antes e depois de cada
símbolo de um caminho original estão cobertos pelos fechos. A atualização dos
finais cobre os trechos lambda de aceitação, inclusive quando não há símbolos.
Por isso os autômatos aceitam a mesma linguagem.

Os estados e o inicial são mantidos. Estados inalcançáveis não são removidos.

## 9. Resultado do exemplo

Os fechos são E(q0)={q0,q1}, E(q1)={q1} e E(q2)={q2}.
O único final resultante continua sendo q2. A tabela resultante é:

| Estado | x | y | 0 |
|---|---|---|---|
| q0 | {q1} | {q2} | {} |
| q1 | {q1} | {q2} | {} |
| q2 | {} | {} | {q2} |

`{}` significa ausência de destinos. O inicial continua sendo q0.
A linguagem desse exemplo é `x*y0*`: zero ou mais x, um y e zero ou mais símbolos 0.

## 10. Responsabilidade de cada função

| Função | Papel |
|---|---|
| `criar` | Valida dimensões e aloca finais e matrizes zeradas com calloc |
| `liberar` | Libera memória e zera a estrutura |
| `linha` | Lê uma linha do teclado; descarta excesso e repete se muito longa |
| `inteiro` | Usa strtol, verifica faixa, estouro e caracteres extras |
| `indice_simbolo` | Busca o índice no alfabeto; retorna m para lambda e -1 se inválido |
| `adicionar_simbolo` | Valida caractere, reserva de lambda e ausência de repetição |
| `cadastrar` | Conduz a entrada interativa de todo o autômato |
| `numero_arquivo` | Lê e valida um inteiro de um arquivo |
| `ler_arquivo` | Abre o caminho informado, valida todos os campos e fecha o arquivo |
| `conjunto` | Imprime um conjunto de estados entre chaves |
| `exibir` | Imprime estados, alfabeto, inicial, finais e transições; o parâmetro mostrar_lambda inclui as transições lambda apenas no original |
| `transformar` | Calcula fechos, finais e transições do AFN equivalente |
| `transformar_afn_lambda_para_afn` | Função pública: submenu, leitura, conversão, exibição e limpeza |

Todas as funções auxiliares são `static`: ficam restritas ao módulo e não
conflitam com nomes iguais em outros arquivos C.

## 11. Erros e gerenciamento de recursos

A leitura de arquivo rejeita dimensões inválidas, símbolos repetidos ou fora
do formato, estados fora da faixa, finais repetidos, transições incompletas,
símbolos não declarados e conteúdo extra. Um erro cancela a conversão daquele
arquivo; nenhum resultado parcial é apresentado como conversão bem-sucedida.

A abertura usa `fopen` e informa falhas com `perror`. O arquivo é fechado tanto
no sucesso quanto em erro. Inteiros são convertidos por `strtol`, com verificação
de `ERANGE` e dos limites permitidos.

As estruturas de entrada e saída começam zeradas. O submenu libera ambas
mesmo quando a leitura ou conversão falha. A conversão também libera suas
matrizes auxiliares. Falhas de alocação são verificadas antes de usar a memória.

## 12. Complexidade

Para n estados e m símbolos:

- Fechos por Warshall: O(n³).
- Construção das transições: O(m × n³) no pior caso.
- Atualização dos finais: O(n²).
- Memória total: O((m+1) × n²), incluindo entrada, saída e auxiliares.

O limite de 100 estados e 26 símbolos mantém a representação por matrizes
adequada ao escopo didático. A quantidade declarada de transições é um inteiro
não negativo limitado a INT_MAX; não há um limite menor de registros de entrada.

## 13. Integração com o grupo

A integração atual ao menu consiste em incluir `afn_lambda.h`, compilar
`afn_lambda.c` junto aos outros fontes e chamar:

```c
transformar_afn_lambda_para_afn();
```

Como o menu principal usa scanf e o módulo usa fgets, `main.c` descarta o
restante da linha da opção antes da chamada. Isso impede que o Enter pendente
seja interpretado como a primeira resposta do submenu.

O módulo agora permite informar o alfabeto, como o conversor dos colegas.
Ainda existem diferenças importantes:

- O módulo lambda aceita 100 estados; a estrutura AFN dos colegas usa máscaras
  de 64 bits e aceita até 64 estados.
- Aqui as transições são matrizes; lá são máscaras de bits.
- Aqui o arquivo declara quantidade de transições; lá lê transições até EOF.
- Aqui a estrutura e a conversão são privadas; a API pública oferece o submenu.

Para uma cadeia automática, será necessário expor uma função de conversão com
estrutura compartilhada ou um adaptador, e alinhar os limites e formatos.
Apenas permitir o mesmo alfabeto não cria essa integração automaticamente.

## 14. Verificação manual

Compile com as opções mostradas na seção 3. Execute o programa, escolha 2
no menu principal e depois 2 para ler `ex_afn_lambda.txt`. Compare a saída
com a tabela da seção 9. Repita escolhendo 1 no submenu para digitar os
mesmos dados, cada campo em uma linha.

Para conferir a propagação de finais por lambda, use dois estados, alfabeto
`x`, inicial 0, final 1 e uma transição `0 l 1`. O resultado deve ter finais
`{q0, q1}` e todas as transições com x vazias.

Confira também arquivos com símbolo repetido, símbolo não declarado, estado
fora da faixa ou transição incompleta: a leitura deve ser rejeitada.
O commit não inclui arquivos de testes nem configuração JSON.

## 15. Resumo para apresentar o módulo

“Minha parte recebe um AFN com lambda, calcula o fecho lambda de todos os
estados, usa esses fechos para reconstruir as transições de cada símbolo e
atualiza os estados finais. O resultado mantém a linguagem e não tem
transições lambda. O alfabeto é informado pelo usuário, e a entrada pode
ser digitada ou carregada de um arquivo. A saída continua sendo um AFN,
que conceitualmente pode ser a entrada da determinização do grupo.”
