#include "minimizar_afd.h"
#include "commons.h"
#include <stdio.h>


/* marca os estados alcancaveis a partir de s */
static void alcancaveis(const AFD *a, int s, int *alc) {
  if (alc[s])
    return;
  alc[s] = 1;
  for (int c = 0; c < a->m; c++)
    alcancaveis(a, a->delta[s][c], alc);
}

int minimizarAFD(const AFD *entrada, AFD *saida) {
  AFD a = *entrada; /* copia de trabalho */

  /* 1) Completa o AFD com um estado morto, se faltar transicao */
  int faltando = 0;
  for (int i = 0; i < a.n; i++)
    for (int c = 0; c < a.m; c++)
      if (a.delta[i][c] < 0)
        faltando = 1;

  if (faltando) {
    if (a.n >= MAX_ESTADOS)
      return 0;
    int morto = a.n;
    a.final[morto] = 0;
    for (int c = 0; c < a.m; c++)
      a.delta[morto][c] = morto;
    for (int i = 0; i < a.n; i++)
      for (int c = 0; c < a.m; c++)
        if (a.delta[i][c] < 0)
          a.delta[i][c] = morto;
    a.n++;
  }

  /* 2) Remove estados inalcancaveis (apenas ignora nos passos seguintes) */
  int alc[MAX_ESTADOS] = {0};
  alcancaveis(&a, a.inicial, alc);

  /* 3) Refinamento de particoes (Moore).
        Comeca com {finais} e {nao finais}; separa estados cujas
        transicoes levam a classes diferentes, ate nada mudar. */
  int cls[MAX_ESTADOS], nova[MAX_ESTADOS];
  int nc = 0, anterior = -1;

  for (int i = 0; i < a.n; i++)
    cls[i] = a.final[i];

  while (1) {
    nc = 0;
    for (int i = 0; i < a.n; i++) {
      if (!alc[i])
        continue;
      nova[i] = -1;
      for (int j = 0; j < i && nova[i] == -1; j++) {
        if (!alc[j] || cls[j] != cls[i])
          continue;
        int igual = 1;
        for (int c = 0; c < a.m; c++)
          if (cls[a.delta[i][c]] != cls[a.delta[j][c]])
            igual = 0;
        if (igual)
          nova[i] = nova[j];
      }
      if (nova[i] == -1)
        nova[i] = nc++;
    }
    for (int i = 0; i < a.n; i++)
      if (alc[i])
        cls[i] = nova[i];
    if (nc == anterior)
      break;
    anterior = nc;
  }

  /* 4) Monta o AFD minimizado, com o estado inicial renumerado para q0 */
  int novoId[MAX_ESTADOS];
  for (int k = 0; k < nc; k++)
    novoId[k] = -1;
  int prox = 0;
  novoId[cls[a.inicial]] = prox++;
  for (int i = 0; i < a.n; i++)
    if (alc[i] && novoId[cls[i]] == -1)
      novoId[cls[i]] = prox++;

  saida->n = nc;
  saida->m = a.m;
  saida->inicial = 0;
  for (int c = 0; c < a.m; c++)
    saida->alfabeto[c] = a.alfabeto[c];
  for (int k = 0; k < nc; k++)
    saida->final[k] = 0;

  for (int i = 0; i < a.n; i++) {
    if (!alc[i])
      continue;
    int k = novoId[cls[i]];
    saida->final[k] = a.final[i];
    for (int c = 0; c < a.m; c++)
      saida->delta[k][c] = novoId[cls[a.delta[i][c]]];
  }
  return 1;
}

void menuMinimizarAFD(void) {
  AFD original, minimo;

  limparTela();
  imprimirCabecalho("MINIMIZAR AFD");

  int opcao, ok;
  char nome[256];

  printf("Como deseja informar o AFD?\n");
  printf("1 - Digitar\n");
  printf("2 - Ler de arquivo .txt\n");
  printf("Opcao: ");
  if (scanf("%d", &opcao) != 1)
    opcao = 1;

  if (opcao == 2) {
    printf("Nome do arquivo: ");
    scanf("%255s", nome);
    ok = lerAFDArquivo(nome, &original);
  } else {
    printf("\n");
    ok = lerAFD(&original);
  }

  if (!ok) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  if (!minimizarAFD(&original, &minimo)) {
    printf("Nao foi possivel minimizar (limite de estados).\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  printf("\n--- AFD original ---\n");
  imprimirAFD(&original);
  printf("\n--- AFD minimizado ---\n");
  imprimirAFD(&minimo);

  /* limpa o ENTER que sobrou do scanf para o pausarTela funcionar */
  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF)
    ;
  pausarTela();
}