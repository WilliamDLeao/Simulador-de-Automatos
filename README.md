# Simulador de Autômatos - Conversor AFN para AFD

Trabalho Final da disciplina de Linguagens Formais e Autômatos (LFA).

Este projeto implementa a conversão de um **Autômato com Finitos Não-Determinístico (AFN)** para um **Autômato com Finitos Determinístico (AFD)** equivalente utilizando o **Algoritmo de Construção de Subconjuntos** (Subset Construction / Powerset Construction via BFS).

---

## 🛠️ Como Compilar e Executar

### Compilação:
```bash
gcc -Wall -Wextra main.c conversor_AFN_para_AFD.c commons.c -o exe
```

### Execução:
```bash
./exe
```

---

## 📄 Formato do Arquivo `.txt` de Entrada

O leitor aceita arquivos com comentários iniciados por `#` ou `//`, e linhas em branco são ignoradas.

### Estrutura do arquivo:
1. **Linha 1:** Quantidade de estados do AFN (estados numerados de `0` a `N-1`).
2. **Linha 2:** Símbolos do alfabeto (separados por espaço, ex: `a b` ou `0 1`).
3. **Linha 3:** Estados de aceitação/finais (ex: `2` ou `1 2`).
4. **Linhas seguintes:** Transições no formato:
   ```text
   <estado_origem> <simbolo> <destino1> <destino2> ...
   ```
   *(Pode-se informar um ou múltiplos destinos na mesma linha).*

### Exemplo (`exemplo_afn_ab.txt` - Palavras terminadas em "ab"):
```text
# Quantidade de estados (0, 1, 2)
3

# Alfabeto
a b

# Estados finais
2

# Transições (origem simbolo destinos...)
0 a 0 1
0 b 0
1 b 2
```

### Exemplo (`exemplo_afn_01.txt` - Palavras contendo "01"):
```text
3
0 1
2
0 0 0 1
0 1 0
1 1 2
2 0 2
2 1 2
```

---

## 🚀 Funcionalidades do Programa

1. **Conversão AFN -> AFD via TXT:** Lê o arquivo `.txt`, constrói os subconjuntos correspondentes via BFS e gera a matriz de transições determinística.
2. **Simulador de Palavras:** Permite testar cadeias de caracteres no AFD resultante, mostrando o passo a passo da transição entre os estados e indicando aceitação ou rejeição.
3. **Visualizador de Tabelas:** Exibe as tabelas formais do AFN e do AFD resultante com a lista detalhada de subconjuntos.
4. **Exportação de AFD:** Salva o AFD resultante em um novo arquivo `.txt` devidamente formatado.
