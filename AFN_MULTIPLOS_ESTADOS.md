# Documentação: Conversor de AFN com Múltiplos Estados para AFN-Lambda

Este módulo implementa a conversão de um **Autômato Finito Não-Determinístico (AFN) com múltiplos estados iniciais** (e opcionalmente múltiplos estados finais) para um **AFN com transições vazias (AFN-$\lambda$)** equivalente, possuindo um **único estado inicial** (e opcionalmente um único estado final).

Ele preenche a **Opção 1** do simulador, estabelecendo a primeira etapa do pipeline completo da disciplina:

$$\text{AFN Múltiplos Iniciais} \xrightarrow{\text{Opção 1}} \text{AFN-}\lambda \xrightarrow{\text{Opção 2}} \text{AFN sem }\lambda \xrightarrow{\text{Opção 3}} \text{AFD} \xrightarrow{\text{Opção 4}} \text{AFD Mínimo}$$

---

## 📌 Fundamentação Teórica

Formalmente, definições estritas de autômatos exigem exatamente **um único estado inicial** $q_0$. Contudo, operações de união de linguagens ou certas modelagens conceituais geram autômatos com um conjunto de estados de partida $Q_0 = \{q_{i_1}, q_{i_2}, \dots, q_{i_k}\}$.

A conversão clássica para a forma canônica de **AFN-$\lambda$** procede da seguinte maneira:

1. **Unificação dos Estados Iniciais:**
   - Cria-se um novo estado inicial $q_{\text{novo\_ini}}$.
   - O novo conjunto de estados passa a ser $Q' = Q \cup \{q_{\text{novo\_ini}}\}$.
   - O estado inicial único torna-se $q_{\text{inicial}} = q_{\text{novo\_ini}}$.
   - Adicionam-se transições vazias ($\lambda$) partindo de $q_{\text{novo\_ini}}$ diretamente para cada um dos múltiplos estados iniciais originais:
     $$\forall q \in Q_0, \quad \delta'(q_{\text{novo\_ini}}, \lambda) \ni q$$
   - Todas as transições originais do autômato em símbolos do alfabeto são preservadas intactas:
     $$\forall q \in Q, \; a \in \Sigma, \quad \delta'(q, a) = \delta(q, a)$$

2. **Unificação dos Estados Finais (Opcional):**
   - Se for desejado também unificar múltiplos estados de aceitação em um único estado final:
   - Cria-se um novo estado final $q_{\text{novo\_fim}}$.
   - Para cada estado final original $f \in F$, remove-se sua marcação de final e adiciona-se uma transição $\lambda$:
     $$\forall f \in F, \quad \delta'(f, \lambda) \ni q_{\text{novo\_fim}}$$
   - O único estado final do autômato passa a ser $q_{\text{novo\_fim}}$.

---

## 📄 Padrão do Arquivo de Entrada (`.txt`)

O leitor é flexível e compatível com o ecossistema do simulador:

```text
<quantidade_estados> <tamanho_alfabeto>
<simbolo_1> <simbolo_2> ... <simbolo_m>
<qtd_iniciais> <inicial_1> <inicial_2> ...
<qtd_finais> <final_1> <final_2> ...
<origem> <simbolo> <destino>
...
```

> **Destaques:**
> - Se a linha de estados iniciais tiver apenas 1 número, o leitor reconhece automaticamente como 1 estado inicial.
> - Suporta comentários iniciados por `#` ou `//` e linhas em branco.
> - O símbolo `l` (L minúsculo) é reservado para transições $\lambda$.

### Exemplo (`exemplo_afn_multiplos.txt`):
```text
4 2
a b
2 0 1
1 3
0 a 2
1 b 2
2 a 3
2 b 3
3 a 3
3 b 3
```

---

## 💻 Estrutura de Código

- [afn_multiplos_estados.h](file:///c:/Users/danni/OneDrive/Área%20de%20Trabalho/Faculdade/6°%20Semestre/Linguagem%20Formais%20e%20Autômatos/Simulador-de-Automatos/afn_multiplos_estados.h): Declaração das estruturas `AFNMultiplo`, `AFNLambda`, funções de I/O e conversão.
- [afn_multiplos_estados.c](file:///c:/Users/danni/OneDrive/Área%20de%20Trabalho/Faculdade/6°%20Semestre/Linguagem%20Formais%20e%20Autômatos/Simulador-de-Automatos/afn_multiplos_estados.c): Implementação com representação eficiente por máscaras de bits (`uint64_t`).
- [exemplo_afn_multiplos.txt](file:///c:/Users/danni/OneDrive/Área%20de%20Trabalho/Faculdade/6°%20Semestre/Linguagem%20Formais%20e%20Autômatos/Simulador-de-Automatos/exemplo_afn_multiplos.txt): Arquivo de teste prático.

---

## 🚀 Compilação

```bash
gcc -Wall -Wextra main.c afd.c minimizar_afd.c commons.c conversor_AFN_para_AFD.c afn_multiplos_estados.c -o exe
./exe
```

No menu principal, selecione a **Opção 1**. O resultado gerado pode ser exportado para um arquivo `.txt` que é 100% compatível com a etapa seguinte de remoção de $\lambda$.
