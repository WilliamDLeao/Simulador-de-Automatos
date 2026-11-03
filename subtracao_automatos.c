#include "subtracao_automatos.h"
#include "commons.h"
#include "minimizar_afd.h"
#include <stdio.h>

/* Retorna o proximo estado de 'a' com o simbolo 'c', tratando estado
   morto/poco. O indice a->n representa o estado morto virtual. */
static int proximoEstado(const AFD *a, int p, char c) {
  if (p < 0 || p >= a->n)
    return a->n; /* Estado morto */

  for (int s = 0; s < a->m; s++) {
    if (a->alfabeto[s] == c) {
      int d = a->delta[p][s];
      return (d >= 0 && d < a->n) ? d : a->n;
    }
  }
  return a->n; /* Simbolo nao presente ou transicao indefinida */
}

int subtrairAFDs(const AFD *a1, const AFD *a2, AFD *saida) {
  if (!a1 || !a2 || !saida)
    return 0;

  /* 1) Alfabeto unificado */
  char alf[MAX_SIMBOLOS];
  int m = 0;
  for (int i = 0; i < a1->m; i++)
    alf[m++] = a1->alfabeto[i];

  for (int i = 0; i < a2->m; i++) {
    int existe = 0;
    for (int k = 0; k < m; k++) {
      if (alf[k] == a2->alfabeto[i]) {
        existe = 1;
        break;
      }
    }
    if (!existe) {
      if (m >= MAX_SIMBOLOS)
        return 0;
      alf[m++] = a2->alfabeto[i];
    }
  }

  /* 2) Mapeamento de pares (p, q) para id de estado no AFD resultante */
  int mapa[MAX_ESTADOS + 1][MAX_ESTADOS + 1];
  for (int i = 0; i <= a1->n; i++) {
    for (int j = 0; j <= a2->n; j++) {
      mapa[i][j] = -1;
    }
  }

  int qp[MAX_ESTADOS], qq[MAX_ESTADOS];
  int ini = 0, fim = 0;

  saida->n = 0;
  saida->m = m;
  saida->inicial = 0;
  for (int s = 0; s < m; s++)
    saida->alfabeto[s] = alf[s];

  for (int i = 0; i < MAX_ESTADOS; i++) {
    saida->final[i] = 0;
    for (int s = 0; s < MAX_SIMBOLOS; s++)
      saida->delta[i][s] = -1;
  }

  /* Estado inicial do produto (q0_1, q0_2) */
  mapa[a1->inicial][a2->inicial] = saida->n++;
  qp[fim] = a1->inicial;
  qq[fim] = a2->inicial;
  fim++;

  /* 3) BFS: constroi apenas os pares alcancaveis */
  while (ini < fim) {
    int u = ini++;
    int p = qp[u], q = qq[u];

    /* Subtracao: aceita se p in F1 E q NAO in F2.
       O estado morto de a2 (q == a2->n) nao e final, entao conta como "nao in
       F2". */
    int f1 = (p < a1->n && a1->final[p]);
    int f2 = (q < a2->n && a2->final[q]);
    saida->final[u] = (f1 && !f2) ? 1 : 0;

    for (int s = 0; s < m; s++) {
      int np = proximoEstado(a1, p, alf[s]);
      int nq = proximoEstado(a2, q, alf[s]);

      if (mapa[np][nq] == -1) {
        if (saida->n >= MAX_ESTADOS)
          return 0;
        int id = saida->n++;
        mapa[np][nq] = id;
        qp[fim] = np;
        qq[fim] = nq;
        fim++;
      }
      saida->delta[u][s] = mapa[np][nq];
    }
  }

  return 1;
}

static int lerAutomatoOpcao(const char *rotulo, AFD *a) {
  int op;
  char nome[256];
  printf("\nComo deseja informar o %s?\n", rotulo);
  printf("1 - Digitar\n");
  printf("2 - Ler de arquivo .txt\n");
  printf("Opcao: ");
  if (scanf("%d", &op) != 1)
    op = 1;

  if (op == 2) {
    printf("Nome do arquivo: ");
    scanf("%255s", nome);
    return lerAFDArquivo(nome, a);
  }
  printf("\n");
  return lerAFD(a);
}

void menuSubtracaoAutomatos(void) {
  AFD a1, a2, subtracao, min;

  limparTela();
  imprimirCabecalho("SUBTRACAO DE AUTOMATOS (AFD)");

  if (!lerAutomatoOpcao("AFD 1", &a1) || !lerAutomatoOpcao("AFD 2", &a2)) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  if (!subtrairAFDs(&a1, &a2, &subtracao)) {
    printf("\nNao foi possivel realizar a subtracao (limite de estados "
           "excedido).\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  printf("\n--- AFD 1 ---\n");
  imprimirAFD(&a1);
  printf("\n--- AFD 2 ---\n");
  imprimirAFD(&a2);
  printf("\n--- AFD Subtracao (L(M1) - L(M2)) ---\n");
  imprimirAFD(&subtracao);

  if (minimizarAFD(&subtracao, &min)) {
    printf("\n--- AFD Subtracao Minimizado ---\n");
    imprimirAFD(&min);
  }

  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF)
    ;
  pausarTela();
}