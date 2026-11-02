# Documentação: Complemento de AFN

Este módulo implementa a operação de **Complemento de Linguagens Reconhecidas por um Autômato Finito Não-Determinístico (AFN)**.

Ela está disponível como a **Opção 9** no menu principal do simulador.

---

## 📌 Fundamentação Teórica: Por que não apenas inverter os estados finais do AFN?

Em um **AFN**, uma palavra $w$ é aceita se **existe pelo menos um caminho** que leva a um estado final ($F$). Se um caminho rejeitar (parar em um estado não-final ou travar por indefinição de transição), mas outro caminho aceitar, a palavra é considerada **aceita**.

Se invertêssemos ingenuamente os estados finais de um AFN:
1. Uma palavra pertencente à linguagem original que possua tanto caminhos de aceitação quanto caminhos de rejeição continuaria sendo aceita pelo "AFN invertido" (pois o caminho antes de rejeição agora levaria a um estado final).
2. Uma palavra cujo processamento travava por transições indefinidas continuaria travando e sendo rejeitada, quando no complemento deveria ser aceita!

Portanto, o procedimento formal canônico de **Linguagens Formais e Teoria dos Autômatos** é:

$$\text{AFN } N \xrightarrow{\text{Subconjuntos}} \text{AFD } D \xrightarrow{\text{Completamento (Estado Morto)}} \text{AFD Total } D' \xrightarrow{F' = Q \setminus F} \text{AFD Complementar } \overline{D}$$

1. **Determinização (Construção de Subconjuntos):** Converte o AFN em um AFD equivalente.
2. **Completamento da Função de Transição:** Se houver transições não definidas para algum símbolo ($\delta(q, a) = -1$), cria-se um **estado morto/poço** ($\delta(q_{\text{morto}}, a) = q_{\text{morto}}, \forall a \in \Sigma$) não-final.
3. **Inversão dos Estados Finais:**
   $$F_{\text{comp}} = Q \setminus F$$
   - Todos os estados antes não-finais (inclusive o estado morto) tornam-se de aceitação (finais).
   - Todos os estados que antes eram de aceitação tornam-se de rejeição (não-finais).
4. **Minimização (Opcional):** Aplica o Algoritmo de Moore (`minimizarAFD`) para obter o menor autômato determinístico equivalente da linguagem complementar.

---

## 📁 Estrutura de Arquivos Criados

- [complemento_afn.h](file:///c:/Users/danni/OneDrive/Área%20de%20Trabalho/Faculdade/6°%20Semestre/Linguagem%20Formais%20e%20Autômatos/Simulador-de-Automatos/complemento_afn.h): Protótipos das funções, estruturas e definições.
- [complemento_afn.c](file:///c:/Users/danni/OneDrive/Área%20de%20Trabalho/Faculdade/6°%20Semestre/Linguagem%20Formais%20e%20Autômatos/Simulador-de-Automatos/complemento_afn.c): Implementação do completamento, inversão de finais, simulação de palavras e menu interativo.
- [main.c](file:///c:/Users/danni/OneDrive/Área%20de%20Trabalho/Faculdade/6°%20Semestre/Linguagem%20Formais%20e%20Autômatos/Simulador-de-Automatos/main.c): Integração da **Opção 9** no menu interativo.

---

## ⚙️ Principais Funções

```c
/* Converte AFN em AFD completo (com estado morto caso falte transicao) */
int determinizarAFNCompleto(const AFN *afn, AFD *dfa_completo);

/* Inverte F' = Q \ F */
void inverterEstadosFinaisAFD(const AFD *entrada, AFD *saida);

/* Converte AFD para estrutura AFN */
int converterAFDparaAFN(const AFD *dfa, AFN *afn);

/* Executa o pipeline completo do complemento */
int complementarAFN(const AFN *afn, AFD *afd_comp, AFN *afn_comp);

/* Simula palavras e confere se sao aceitas */
int simularPalavraAFD(const AFD *a, const char *palavra);

/* Menu interativo */
void menuComplementoAFN(void);
```

---

## 🚀 Como Compilar e Executar

```bash
gcc -Wall -Wextra main.c afd.c minimizar_afd.c commons.c conversor_AFN_para_AFD.c afn_multiplos_estados.c complemento_afn.c -o exe
./exe
```

No menu principal, selecione a **Opção 9**.
