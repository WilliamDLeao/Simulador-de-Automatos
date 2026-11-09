# Simulador de Autômatos Finitos

Projeto desenvolvido para a disciplina de **Linguagens Formais e Autômatos (6º Semestre)**. O sistema é um simulador interativo em C para criação, conversão, minimização e operações algébricas sobre autômatos finitos.

---

## 🚀 Compilação e Execução

### Windows (Prompt de Comando ou PowerShell)
```cmd
run.bat
```
ou manualmente com GCC:
```cmd
gcc -Wall -Wextra *.c -o exe.exe
.\exe.exe
```

### Linux / macOS
```bash
gcc -std=c11 -Wall -Wextra -pedantic -g *.c -o simulador
./simulador
```
ou utilizando o `Makefile`:
```bash
make run
```

---

## 🔄 Fluxo de Processamento

O simulador suporta o pipeline completo de conversão e simplificação de autômatos:

$$\text{AFN com Múltiplos Iniciais (1)} \xrightarrow{} \text{AFN-}\lambda \text{ (2)} \xrightarrow{} \text{AFN sem }\lambda \text{ (3)} \xrightarrow{} \text{AFD (4)} \xrightarrow{} \text{AFD Mínimo}$$

Além de operações entre linguagens: **União (6)**, **Interseção (7)**, **Subtração (8)** e **Complemento (9)**.

---

## 📄 Padrão dos Arquivos de Entrada (`.txt`)

O formato padrão aceito pelos leitores do simulador segue a estrutura:

```text
<qtd_estados> <tam_alfabeto>
<simbolo_1> <simbolo_2> ... <simbolo_m>
<estado_inicial>
<qtd_finais> <final_1> <final_2> ...
<origem> <simbolo> <destino>
...
```

- **Símbolos:** Caracteres ASCII visíveis (exceto espaços). A letra `l` (L minúsculo) é reservada para transições $\lambda$.
- **Estados:** Identificados numericamente de `0` a `n-1`.
- **Comentários:** Linhas iniciadas com `#` ou `//` e linhas vazias são ignoradas na maioria dos leitores.

---

## 🛠️ Funcionalidade de Cada Função por Módulo

### 1. Ponto de Entrada (`main.c`)
- `main()`: Loop do menu principal; gerencia a seleção do usuário e direciona a execução para o submenu correspondente de cada operação.

---

### 2. Opção 1 — AFN com Múltiplos Estados para AFN-$\lambda$ (`afn_multiplos_estados.c` / `.h`)
Converte autômatos com múltiplos estados iniciais/finais em um AFN-$\lambda$ com inicial único.

- `lerAFNMultiplo(AFNMultiplo *a)`: Lê os dados de um autômato com múltiplos estados via terminal interativo.
- `lerAFNMultiploArquivo(const char *nome, AFNMultiplo *a)`: Carrega a definição de um AFN com múltiplos estados a partir de arquivo `.txt`.
- `imprimirAFNMultiplo(const AFNMultiplo *a)`: Exibe no terminal os múltiplos estados iniciais, finais e as transições do AFN.
- `imprimirAFNLambda(const AFNLambda *a)`: Imprime o AFN-$\lambda$ gerado, exibindo as transições do alfabeto e transições vazias ($\lambda$).
- `salvarAFNLambdaArquivo(const char *nome, const AFNLambda *a)`: Exporta o AFN-$\lambda$ gerado para um arquivo `.txt` formatado para a Opção 2.
- `converterMultiplosParaLambda(const AFNMultiplo *entrada, AFNLambda *saida, int unificar_finais)`: Cria um novo estado inicial único com transições $\lambda$ para todos os iniciais originais; opcionalmente unifica múltiplos finais em um único estado final via $\lambda$.
- `menuAFNMultiplosParaLambda(void)`: Submenu interativo para carregar, converter, visualizar e salvar o AFN-$\lambda$.

---

### 3. Opção 2 — AFN-$\lambda$ para AFN sem $\lambda$ (`afn_lambda.c` / `.h`)
Remove transições vazias preservando a linguagem reconhecida (inclusive aceitação de palavra vazia).

- `transformar_afn_lambda_para_afn(void)`: Função principal do módulo (chamada pelo `main`); controla o submenu, leitura, conversão e exibição.
- `transformar(Automato *a, Automato *res)`: Executa o algoritmo de conversão:
  - Calcula o fecho-$\lambda$ de cada estado usando o algoritmo de Warshall ($O(n^3)$).
  - Reconstrói a matriz de transições sem $\lambda$: $\delta'(q, x) = \text{fecho}(\text{mover}(\text{fecho}(q), x))$.
  - Marca como finais os estados cujo fecho-$\lambda$ alcance algum estado final original.
- `criar(int n, int m, Automato *a)`: Aloca dinamicamente as matrizes de adjacência e o vetor de estados finais.
- `liberar(Automato *a)`: Libera a memória alocada para as transições e zera a estrutura.
- `cadastrar(Automato *a)`: Conduz a entrada manual de dados do AFN-$\lambda$ pelo teclado.
- `ler_arquivo(const char *caminho, Automato *a)`: Abre, lê e valida campos e transições de um arquivo `.txt`.
- `exibir(const Automato *a, int mostrar_lambda)`: Imprime estados, alfabeto, inicial, finais e transições no console.

---

### 4. Opção 3 — Conversão de AFN para AFD (`conversor_AFN_para_AFD.c` / `.h`)
Determina um autômato finito através da construção de subconjuntos.

- `lerAFN(AFN *a)`: Lê um AFN (sem $\lambda$) informado diretamente pelo teclado.
- `lerAFNArquivo(const char *nome, AFN *a)`: Lê a estrutura de um AFN a partir de arquivo de texto.
- `imprimirAFN(const AFN *a)`: Imprime a tabela de transições e propriedades do AFN no terminal.
- `converterAFNparaAFD(const AFN *afn, AFD *dfa)`: Executa o **Algoritmo de Construção de Subconjuntos** (*Subset Construction* via BFS), mapeando combinações de estados do AFN em estados únicos do AFD.
- `menuConversorAFNparaAFD(void)`: Submenu para carregar o AFN, efetuar a determinização e exibir o AFD equivalente.

---

### 5. Opção 4 — Minimização de AFD (`minimizar_afd.c` / `.h` e `afd.c` / `.h`)
Reduz o AFD para o menor número de estados possíveis usando partição de equivalência.

#### Módulo Base do AFD (`afd.c` / `.h`)
- `lerAFD(AFD *a)`: Lê um AFD via teclado.
- `lerAFDArquivo(const char *nome, AFD *a)`: Lê um AFD a partir de arquivo `.txt`.
- `imprimirAFD(const AFD *a)`: Exibe a tabela de transição e definições formais do AFD.

#### Minimizador (`minimizar_afd.c` / `.h`)
- `minimizarAFD(const AFD *entrada, AFD *saida)`: Aplica o **Algoritmo de Moore** para particionar os estados em classes de equivalência indistinguíveis, eliminando redundâncias e estados inalcançáveis.
- `menuMinimizarAFD(void)`: Submenu para carregar o AFD, executar a minimização e imprimir a versão minimizada.

---

### 6. Opção 6 — União de Autômatos (`uniao_automatos.c` / `.h`)
Calcula a união das linguagens de dois AFDs: $L = L(M_1) \cup L(M_2)$.

- `unirAFDs(const AFD *a1, const AFD *a2, AFD *saida)`: Constrói o AFD produto em paralelo via BFS com o alfabeto unificado ($\Sigma_1 \cup \Sigma_2$). Um par $(p, q)$ é final se $p \in F_1 \lor q \in F_2$.
- `menuUniaoAutomatos(void)`: Submenu para carregar os dois autômatos, calcular a união, exibir o autômato resultante e aplicar minimização automática.

---

### 7. Opção 7 — Interseção de Autômatos (`interseccao_automatos.c` / `.h`)
Calcula a interseção de linguagens: $L = L(M_1) \cap L(M_2)$ para pares de AFDs ou AFNs.

- `intersectarAFDs(const AFD *a1, const AFD *a2, AFD *saida)`: Executa o produto cartesiano entre dois AFDs; um par $(p, q)$ é final se $p \in F_1 \land q \in F_2$.
- `intersectarAFNs(const AFN *a1, const AFN *a2, AFD *saida)`: Recebe dois AFNs, determiniza-os e calcula a interseção dos AFDs resultantes.
- `menuInterseccaoAutomatos(void)`: Submenu completo com opções para carregar autômatos, testar aceitação de palavras interativamente, salvar resultados e inspecionar etapas intermediárias.

---

### 8. Opção 8 — Subtração de Autômatos (`subtracao_automatos.c` / `.h`)
Calcula a diferença entre duas linguagens regulares: $L = L(M_1) \setminus L(M_2)$.

- `subtrairAFDs(const AFD *a1, const AFD *a2, AFD *saida)`: Gera o produto cartesiano em paralelo onde um par $(p, q)$ é final se $p \in F_1 \land q \notin F_2$.
- `menuSubtracaoAutomatos(void)`: Submenu que gerencia a leitura dos autômatos, calcula a subtração e exibe o AFD resultante minimizado.

---

### 9. Opção 9 — Complemento de AFN (`complemento_afn.c` / `.h`)
Calcula o complemento da linguagem de um AFN: $L(\overline{M}) = \Sigma^* \setminus L(M)$.

- `determinizarAFNCompleto(const AFN *afn, AFD *dfa_completo)`: Converte o AFN em AFD e garante que a função de transição seja total, inserindo um estado morto/poço caso haja transições indefinidas.
- `inverterEstadosFinaisAFD(const AFD *entrada, AFD *saida)`: Inverte os estados de aceitação ($F' = Q \setminus F$), tornando estados finais em não-finais e vice-versa.
- `converterAFDparaAFN(const AFD *dfa, AFN *afn)`: Converte a representação de um AFD para a estrutura de AFN correspondente.
- `complementarAFN(const AFN *afn, AFD *afd_comp, AFN *afn_comp)`: Orquestra o pipeline completo: determinização total $\to$ estado morto $\to$ inversão de finais.
- `simularPalavraAFD(const AFD *a, const char *palavra)`: Testa o processamento de uma palavra no AFD, retornando 1 (aceita), 0 (rejeitada) ou -1 (símbolo inválido).
- `salvarAFDEmArquivo(const char *nome, const AFD *a)`: Salva um AFD em arquivo `.txt`.
- `salvarAFNEmArquivo(const char *nome, const AFN *a)`: Salva um AFN em arquivo `.txt`.
- `menuComplementoAFN(void)`: Submenu para carregar AFN, calcular complemento, testar palavras e salvar saídas.

---

### 10. Módulos Utilitários

#### Funções Comuns (`commons.c` / `.h`)
- `limparTela()`: Limpa o terminal de forma compatível com Windows (`cls`) e Linux/POSIX (`clear`).
- `imprimirCabecalho(const char *titulo)`: Exibe um cabeçalho formatado para os menus.
- `pausarTela()`: Pausa a execução e aguarda `Enter` antes de prosseguir.

#### Leitor de JSON (`json_reader.c` / `.h`)
- `lerArquivoJson(const char *caminho, ArquivoJsonAutomatos *res)`: Faz o parse de arquivos `.json` estruturados contendo 1 ou 2 autômatos.
- `automatoJsonParaAFD(...)` / `automatoJsonParaAFN(...)` / `automatoJsonParaAFNMultiplo(...)`: Converte os dados intermediários do JSON para os tipos de dados nativos do projeto.
- `lerAFDJson(...)` / `lerAFNJson(...)` / `lerAFNMultiploJson(...)`: Permite carregar um autômato específico de um arquivo `.json`.
- `lerDoisAFDsJson(...)` / `lerDoisAFNsJson(...)`: Lê simultaneamente dois autômatos de um mesmo arquivo `.json` (usado em união, interseção e subtração).
