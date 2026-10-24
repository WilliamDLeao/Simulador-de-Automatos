# Documentação: Conversor de AFN para AFD

Este módulo implementa a conversão de um **Autômato Finito Não-Determinístico (AFN)** para um **Autômato Finito Determinístico (AFD)** equivalente utilizando o **Algoritmo de Construção de Subconjuntos** (*Subset Construction* via busca em largura - BFS).

---

## 📌 Interface Enxuta

O módulo segue o mesmo padrão de simplicidade e modularidade de `minimizar_afd.h` e `afd.h`:

```c
/* conversor_AFN_para_AFD.h */
int lerAFN(AFN *a);                          /* le do teclado, retorna 1 se ok */
int lerAFNArquivo(const char *nome, AFN *a); /* le de um .txt, retorna 1 se ok */
void imprimirAFN(const AFN *a);

int converterAFNparaAFD(const AFN *afn, AFD *dfa);
void menuConversorAFNparaAFD(void);
```

---

## 📄 Padrão de Leitura

O leitor segue o formato padrão de `ex_leitura.txt` e `lerAFDArquivo`:

```text
<n_estados> <tam_alfabeto>
<simbolo_1> <simbolo_2> ... <simbolo_m>
<estado_inicial>
<qtd_finais> <final_1> <final_2> ...
<origem> <simbolo> <destino>
...
```

### Exemplo (`exemplo_afn_ab.txt` - Palavras terminadas em "ab"):
```text
3 2
a b
0
1 2
0 a 0
0 a 1
0 b 0
1 b 2
```

> **Características:**
> - Comentários iniciados por `#` ou `//` e linhas vazias são ignorados.
> - Suporta transições unitárias (`0 a 0` e `0 a 1`) ou com múltiplos destinos (`0 a 0 1`).

---

## 🧠 Funcionamento do Algoritmo

1. **Estado Inicial:** O primeiro estado do AFD $S_0$ recebe o conjunto $\{q_{inicial}\}$ do AFN e é colocado na fila BFS.
2. **Busca em Largura (BFS):**
   - Retira o subconjunto $S_{atual}$ da fila.
   - Para cada símbolo do alfabeto, calcula a união dos destinos:
     $$\text{dest} = \bigcup_{q \in S_{atual}} \delta(q, \text{simbolo})$$
   - Se $\text{dest}$ for vazio, não cria transição.
   - Se $\text{dest}$ for um conjunto novo, atribui um novo ID determinístico ($S_1, S_2, \dots$), define se é final e o enfileira.
   - Se já existir, conecta a transição diretamente.
3. **Aceitação:** Qualquer estado $S$ que contenha ao menos um estado final do AFN é marcado como estado final no AFD.

---

## 💻 Compilação e Execução

```bash
gcc -Wall -Wextra main.c afd.c minimizar_afd.c commons.c conversor_AFN_para_AFD.c -o exe
./exe
```

No menu principal, selecione a **Opção 3** para executar o conversor.
