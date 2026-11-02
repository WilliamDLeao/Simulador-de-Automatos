# Documentação: União de Autômatos

Este módulo implementa a operação de **União de Autômatos Finitos Determinísticos (AFD)**, correspondente à **Opção 6** do menu principal.

---

## 📌 Fundamentação Teórica: Produto Cartesiano (Construção em Paralelo)

Dadas duas linguagens regulares reconhecidas por $M_1 = (Q_1, \Sigma_1, \delta_1, q_{0,1}, F_1)$ e $M_2 = (Q_2, \Sigma_2, \delta_2, q_{0,2}, F_2)$, a linguagem união $L = L(M_1) \cup L(M_2)$ é obtida pela construção em paralelo:

1. **Alfabeto Unificado:** $\Sigma = \Sigma_1 \cup \Sigma_2$.
2. **Estados do Produto:** Pares $(p, q) \in Q_1 \times Q_2$ alcançáveis a partir de $(q_{0,1}, q_{0,2})$ (gerados via BFS). Transições indefinidas ou símbolos ausentes são direcionados a um estado morto virtual.
3. **Função de Transição:**
   $$\delta((p, q), a) = (\delta_1(p, a), \delta_2(q, a)), \quad \forall a \in \Sigma$$
4. **Condição de Aceitação da União:**
   $$(p, q) \in F \iff p \in F_1 \lor q \in F_2$$
5. **Minimização (Moore):** O AFD união resultante é minimizado automaticamente.

---

## 📁 Arquivos

- [uniao_automatos.h](file:///home/monstrinholinux/Documentos/Facul/Simulador-de-Automatos/uniao_automatos.h): Declaração da função `unirAFDs` e do menu.
- [uniao_automatos.c](file:///home/monstrinholinux/Documentos/Facul/Simulador-de-Automatos/uniao_automatos.c): Implementação concisa do algoritmo e interface.
- [exemplo_uniao_m1.txt](file:///home/monstrinholinux/Documentos/Facul/Simulador-de-Automatos/exemplo_uniao_m1.txt): Exemplo de AFD 1 (palavras começando com 'a').
- [exemplo_uniao_m2.txt](file:///home/monstrinholinux/Documentos/Facul/Simulador-de-Automatos/exemplo_uniao_m2.txt): Exemplo de AFD 2 (palavras terminando com 'b').

---

## 🚀 Compilação e Execução

```bash
gcc -Wall -Wextra main.c afd.c minimizar_afd.c commons.c conversor_AFN_para_AFD.c afn_multiplos_estados.c complemento_afn.c uniao_automatos.c -o exe
./exe
```
