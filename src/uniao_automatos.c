#include "uniao_automatos.h"
#include "commons.h"
#include "exportar_automato.h"
#include "minimizar_afd.h"
#include "json_reader.h"
#include <stdio.h>
#include <string.h>

/* Retorna o proximo estado de 'a' com o simbolo 'c', tratando estado morto/poco */
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

int unirAFDs(const AFD *a1, const AFD *a2, AFD *saida) {
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

    /* Uniao: aceita se p in F1 OU q in F2 */
    int f1 = (p < a1->n && a1->final[p]);
    int f2 = (q < a2->n && a2->final[q]);
    saida->final[u] = (f1 || f2) ? 1 : 0;

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
  printf("3 - Ler de arquivo .json\n");
  printf("Opcao: ");
  if (scanf("%d", &op) != 1)
    op = 1;

  if (op == 2) {
    printf("Nome do arquivo: ");
    scanf("%255s", nome);
    if (strstr(nome, ".json") != NULL) {
      return lerAFDJson(nome, a, 0);
    }
    return lerAFDArquivo(nome, a);
  } else if (op == 3) {
    printf("Nome do arquivo .json: ");
    scanf("%255s", nome);
    return lerAFDJson(nome, a, 0);
  }
  printf("\n");
  return lerAFD(a);
}

void menuUniaoAutomatos(void) {
  AFD a1, a2, uniao, min;

  limparTela();
  imprimirCabecalho("UNIAO DE AUTOMATOS (AFD)");

  printf("Como deseja carregar os automatos para a uniao?\n");
  printf("1 - Ler ambos de um unico arquivo .json (ex: exemplo_leitura.json)\n");
  printf("2 - Informar AFD 1 e AFD 2 separadamente (teclado / .txt / .json)\n");
  printf("Opcao: ");
  int modo = 1;
  if (scanf("%d", &modo) != 1)
    modo = 1;

  int ok = 0;
  if (modo == 1) {
    char nome[256];
    printf("Nome do arquivo .json (ex: exemplo_leitura.json): ");
    scanf("%255s", nome);
    ok = lerDoisAFDsJson(nome, &a1, &a2);
  } else {
    ok = lerAutomatoOpcao("AFD 1", &a1) && lerAutomatoOpcao("AFD 2", &a2);
  }

  if (!ok) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  if (!unirAFDs(&a1, &a2, &uniao)) {
    printf("\nNao foi possivel realizar a uniao (limite de estados excedido).\n");
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
  printf("\n--- AFD Uniao (L(M1) U L(M2)) ---\n");
  imprimirAFD(&uniao);

  if (minimizarAFD(&uniao, &min)) {
    printf("\n--- AFD Uniao Minimizado ---\n");
    imprimirAFD(&min);
    oferecerExportacaoAFD(&min, "afd_uniao_minimizado");
  } else {
    oferecerExportacaoAFD(&uniao, "afd_uniao");
  }

  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF)
    ;
  pausarTela();
}
