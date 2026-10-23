#include "afd.h"
#include <stdio.h>


/* guarda a transicao origem --sim--> destino. Retorna 0 se invalida. */
static int adicionarTransicao(AFD *a, int o, char sim, int d) {
  int s = -1;
  for (int k = 0; k < a->m; k++)
    if (a->alfabeto[k] == sim)
      s = k;

  if (o < 0 || o >= a->n || d < 0 || d >= a->n) {
    printf("Estado fora do intervalo em: %d %c %d\n", o, sim, d);
    return 0;
  }
  if (s < 0) {
    printf("Simbolo '%c' nao pertence ao alfabeto.\n", sim);
    return 0;
  }
  if (a->delta[o][s] != -1 && a->delta[o][s] != d) {
    printf("Nao e um AFD: q%d com '%c' tem mais de um destino (q%d e q%d).\n",
           o, sim, a->delta[o][s], d);
    return 0;
  }
  a->delta[o][s] = d;
  return 1;
}

/* Leitura generica: 'f' pode ser stdin ou um arquivo.
   'msg' = 1 mostra as perguntas (teclado), 0 le em silencio (arquivo). */
static int lerAFDde(FILE *f, AFD *a, int msg) {
  int nf, fin;

  if (msg)
    printf("Quantidade de estados (estados serao 0..n-1): ");
  if (fscanf(f, "%d", &a->n) != 1 || a->n < 1 || a->n >= MAX_ESTADOS) {
    printf("Quantidade de estados invalida (1 a %d).\n", MAX_ESTADOS - 1);
    return 0;
  }

  if (msg)
    printf("Tamanho do alfabeto: ");
  if (fscanf(f, "%d", &a->m) != 1 || a->m < 1 || a->m > MAX_SIMBOLOS) {
    printf("Tamanho de alfabeto invalido (1 a %d).\n", MAX_SIMBOLOS);
    return 0;
  }

  if (msg)
    printf("Simbolos (separados por espaco): ");
  for (int s = 0; s < a->m; s++) {
    if (fscanf(f, " %c", &a->alfabeto[s]) != 1) {
      printf("Erro ao ler o alfabeto.\n");
      return 0;
    }
  }

  if (msg)
    printf("Estado inicial: ");
  if (fscanf(f, "%d", &a->inicial) != 1 || a->inicial < 0 ||
      a->inicial >= a->n) {
    printf("Estado inicial invalido.\n");
    return 0;
  }

  for (int i = 0; i < a->n; i++)
    a->final[i] = 0;
  if (msg)
    printf("Quantidade de estados finais: ");
  if (fscanf(f, "%d", &nf) != 1 || nf < 0 || nf > a->n) {
    printf("Quantidade de finais invalida.\n");
    return 0;
  }
  if (msg && nf > 0)
    printf("Estados finais: ");
  for (int i = 0; i < nf; i++) {
    if (fscanf(f, "%d", &fin) != 1 || fin < 0 || fin >= a->n) {
      printf("Estado final invalido.\n");
      return 0;
    }
    a->final[fin] = 1;
  }

  /* todas as transicoes comecam inexistentes (-1) */
  for (int i = 0; i < a->n; i++)
    for (int s = 0; s < a->m; s++)
      a->delta[i][s] = -1;

  if (msg) {
    int qtd;
    printf("Quantidade de transicoes: ");
    if (fscanf(f, "%d", &qtd) != 1 || qtd < 0) {
      printf("Quantidade de transicoes invalida.\n");
      return 0;
    }
    if (qtd > 0)
      printf(
          "Digite cada transicao como: origem simbolo destino (ex: 0 a 1)\n");
    for (int t = 0; t < qtd; t++) {
      int o, d;
      char sim;
      printf("Transicao %d: ", t + 1);
      if (fscanf(f, "%d %c %d", &o, &sim, &d) != 3 ||
          !adicionarTransicao(a, o, sim, d))
        return 0;
    }
  } else {
    /* arquivo: le transicoes ate o fim */
    int o, d, r;
    char sim;
    while ((r = fscanf(f, " %d %c %d", &o, &sim, &d)) != EOF) {
      if (r != 3) {
        printf("Transicao mal formatada no arquivo (use: origem simbolo "
               "destino).\n");
        return 0;
      }
      if (!adicionarTransicao(a, o, sim, d))
        return 0;
    }
  }
  return 1;
}

int lerAFD(AFD *a) { return lerAFDde(stdin, a, 1); }

int lerAFDArquivo(const char *nome, AFD *a) {
  FILE *f = fopen(nome, "r");
  if (!f) {
    printf("Nao foi possivel abrir o arquivo '%s'.\n", nome);
    return 0;
  }
  int ok = lerAFDde(f, a, 0);
  fclose(f);
  return ok;
}

void imprimirAFD(const AFD *a) {
  printf("Estados   : %d (q0..q%d)\n", a->n, a->n - 1);
  printf("Alfabeto  : ");
  for (int s = 0; s < a->m; s++)
    printf("%c ", a->alfabeto[s]);
  printf("\nInicial   : q%d\n", a->inicial);
  printf("Finais    : ");
  for (int i = 0; i < a->n; i++)
    if (a->final[i])
      printf("q%d ", i);
  printf("\nTransicoes:\n");
  for (int i = 0; i < a->n; i++)
    for (int s = 0; s < a->m; s++) {
      if (a->delta[i][s] < 0)
        printf("  delta(q%d, %c) = -\n", i, a->alfabeto[s]);
      else
        printf("  delta(q%d, %c) = q%d\n", i, a->alfabeto[s], a->delta[i][s]);
    }
}