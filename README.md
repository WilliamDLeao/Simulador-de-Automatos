# Simulador de autômatos

## Compilar e executar (Linux, GCC)

```bash
gcc -std=c11 -Wall -Wextra -pedantic -g main.c afd.c commons.c minimizar_afd.c conversor_AFN_para_AFD.c afn_lambda.c -o simulador
./simulador
```

No VS Code, use os comandos acima no terminal integrado.

## AFN lambda para AFN

No menu principal, escolha **2**. Depois escolha **1** para digitar ou **2**
para carregar um arquivo. Informe de 1 a 26 símbolos distintos: letras, dígitos ou pontuação ASCII
visível, cada um com um único caractere. Espaços, acentos e `l` não são
permitidos como símbolos; `l` continua reservado para lambda.
Use `l` (L minúsculo) para lambda e números de 0 a n−1 para os estados.
São permitidos de 1 a 100 estados, zero finais e zero transições.
Para múltiplos destinos, informe uma transição por destino.

### Entrada por arquivo

Copie seu `.txt` para a pasta do projeto e digite seu nome, por exemplo
`ex_afn_lambda.txt`. Também é possível informar um caminho completo, inclusive
com espaços (sem aspas). O arquivo contém, nesta ordem:

1. Quantidade de estados.
2. Quantidade de símbolos do alfabeto.
3. Símbolos do alfabeto, separados por espaços.
4. Número do estado inicial.
5. Quantidade de estados finais.
6. Números dos estados finais, sem repetições (nenhum se a quantidade for zero).
7. Quantidade de transições.
8. Uma tripla `origem simbolo destino` por transição.

Exemplo:

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

Espaços e quebras de linha separam os dados; não inclua comentários.
No modo teclado, os mesmos dados são solicitados individualmente, incluindo
um símbolo por linha. Arquivos antigos precisam receber a quantidade e os
símbolos do alfabeto após a quantidade de estados. A quantidade de transições
continua obrigatória; este formato ainda difere do conversor AFN → AFD do grupo.

O exemplo gera finais `{q2}` e transições de q0 e q1 com `x` para `{q1}`
e com `y` para `{q2}`. De q2, `0` leva a `{q2}`. As demais transições são vazias.

### Método

O fecho lambda de cada estado inclui ele próprio e todos os estados alcançáveis
por lambda. O algoritmo de Warshall calcula esses fechos, inclusive em ciclos.
Para cada símbolo x, a conversão usa:

`delta_novo(q,x) = fecho_lambda(mover(fecho_lambda(q),x))`.

Um estado torna-se final se seu fecho contém algum final original, preservando
a aceitação da palavra vazia. O estado inicial é mantido. O resultado é um AFN
sem lambda, exibido no terminal; não é uma determinização nem minimização.
A opção 4 continua disponível para minimizar AFDs.

## Documentação

A explicação completa do módulo está em [AFN_LAMBDA.md](AFN_LAMBDA.md).
