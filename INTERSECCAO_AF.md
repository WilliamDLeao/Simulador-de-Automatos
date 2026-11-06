# Opção 7 — Interseção de dois AFs (AFN/AFD)

Responsável: Thallys. Calcula L(M1) ∩ L(M2): a palavra deve ser aceita pelos
**dois** autômatos. Cada entrada pode ser AFN ou AFD, sem escolher o tipo.
Assim como o complemento (9), aceita um inicial e não aceita transições lambda.

## Executar e experimentar

Na raiz do projeto, com GCC e make no PATH: `make run`.
Se necessário, no PowerShell, configure antes:

```powershell
$env:Path = "$env:LOCALAPPDATA\Programs\w64devkit\bin;$env:Path"
make run
```

Encerre o simulador com 0 antes de recompilar: Windows bloqueia executáveis em uso.
No VS Code, Ctrl+Shift+B compila; F5 depura.

1. Escolha **7**.
2. Escolha **2**, arquivo `exemplo_interseccao_afn_m1.txt`.
3. Escolha **2**, arquivo `exemplo_interseccao_afn_m2.txt`.
4. Nos resultados, escolha **1** para testar palavras.
5. Teste `aab`, `ab`, `aa` e `_`; digite `fim` para sair do teste.
6. Escolha **0** para voltar ao menu principal.

M1 reconhece palavras terminadas em `ab`; M2, palavras contendo `aa`.
Ambos têm dois destinos para `a` em q0, demonstrando não determinismo real.

| Palavra | M1 | M2 | Interseção |
| --- | --- | --- | --- |
| aab | aceita | aceita | aceita |
| ab | aceita | rejeita | rejeita |
| aa | rejeita | aceita | rejeita |
| vazia | rejeita | rejeita | rejeita |

Resultado: **6 estados, inicial q0, final q4**. Minimizado: **5 estados,
inicial q0, final q3**.

## Resultados e opções

Por padrão aparece apenas o AFD da interseção. A opção 5 exibe os dois AFs originais, seus AFDs determinizados e completos,
o AFN equivalente e o AFD minimizado por Moore. Cada exibição informa estados,
alfabeto, inicial, finais e transições.

```text
1 - Testar palavras em M1, M2 e na Interseccao
2 - Salvar AFD Interseccao em arquivo .txt
3 - Salvar AFN Interseccao equivalente em arquivo .txt
4 - Salvar AFD Interseccao minimizado em arquivo .txt
5 - Ver etapas da construcao
0 - Voltar ao menu principal
```

O teste verifica `resultado == (M1 && M2)`. ENTER, `_`, `lambda` ou `vazio`
representam palavra vazia; `fim` volta. Para testar esses textos literalmente,
use `=fim`, `=_`, etc. Símbolo fora da união dos alfabetos é inválido; ausente
somente em um operando representa rejeição nesse operando.

Exportações usam o formato do grupo: AFD pode ser lido pela minimização (4);
AFN, pela conversão (3), complemento (9) e interseção (7). O AFN equivalente tem
destinos unitários, o que é válido. Caminhos com espaços funcionam sem aspas;
caminhos relativos partem da pasta de execução. Arquivos existentes são
preservados: escolha outro nome ao salvar.
Para carregar nos menus antigos (3, 4 e 9), use caminhos sem espaços: os
seletores de arquivo desses módulos ainda leem o nome com `scanf`.

## Entrada

O programa explica a interseção uma vez e mostra, antes de cada arquivo, um nome de
exemplo. Na leitura, caminhos podem ter aspas simples ou duplas. ENTER vazio,
arquivo ausente ou formato inválido permitem tentar novamente na mesma etapa;
digite `0` no campo do caminho para cancelar. Após carregar, uma confirmação
informa quantos estados e símbolos foram lidos.


Um arquivo por autômato:

```text
3 2
a b
0
1 2
0 a 0 1
0 b 0
1 b 2
```

Linhas: quantidade de estados/tamanho do alfabeto; símbolos; inicial;
quantidade de finais e lista de finais; transições até EOF, sem contador.
Use `0` na linha de finais para nenhum final. Estados numerados de 0 a n−1.
Cada transição é origem, símbolo e um ou mais destinos. Pode repetir linhas
com destinos diferentes. `0 a -` representa ausência de destinos. Aceita
linhas vazias e comentários em linha própria começando por `#` ou `//`.
Símbolos são caracteres únicos e distintos; espaços e vírgulas são separadores.
Não aceita transições lambda nem rótulos multicaractere.

No teclado, o leitor compartilhado pede a quantidade de transições e recebe
um destino por transição; repita origem/símbolo para adicionar destinos.

## Algoritmo, integração e limites

Determiniza cada AFN por subconjuntos e completa com poço quando necessário.
O produto percorre pares alcançáveis em largura: final se **ambos** forem finais.
Une alfabetos, associando colunas por símbolo, com poço para símbolos ausentes.
Aceita palavra vazia se ambos os iniciais aceitam. Sem finais alcançáveis, a
linguagem é vazia. A minimização preserva a linguagem, podendo renumerar estados.

- Entrada do menu: 1 a 64 estados por AF, inclusive AFDs.
- Determinização/completamento/produto: até 100 estados; união: até 26 símbolos.
- AFN equivalente: até 64 estados; acima disso, exibição/exportação AFN ficam
  indisponíveis, mantendo AFD e minimização disponíveis.
- Exportação AFD: até 99 estados, limite do leitor AFD do grupo. Para produto
  com 100, tente exportar o minimizado.
- Teste interativo: até 1022 caracteres por palavra; linhas maiores são rejeitadas.

Não trunca resultados ao atingir limites. As APIs retornam 1 no sucesso, 0 em
erro, mantendo saída intacta em erro:

```c
int intersectarAFDs(const AFD *a1, const AFD *a2, AFD *saida);
int intersectarAFNs(const AFN *a1, const AFN *a2, AFD *saida);
void menuInterseccaoAutomatos(void);
```

A API AFD continua disponível, aceitando até 100 estados por entrada. AFNs da
API devem manter `final[]` e `finais_mask` consistentes. Dependências:
`afd`, `conversor_AFN_para_AFD`, `complemento_afn` (determinização completa,
simulação e exportação), `minimizar_afd` e `commons`. Os algoritmos dos colegas
não foram alterados. Integração: incluir cabeçalho no main e chamar no case 7.
