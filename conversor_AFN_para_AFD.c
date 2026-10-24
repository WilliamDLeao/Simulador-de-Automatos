#include "conversor_AFN_para_AFD.h"
#include "commons.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int obterIndiceSimbolo(const AFN *a, char c) {
  for (int i = 0; i < a->m; i++) {
    if (a->alfabeto[i] == c)
      return i;
  }
  return -1;
}

static int adicionarTransicao(AFN *a, int o, char sim, int d) {
  int s = obterIndiceSimbolo(a, sim);
  if (o < 0 || o >= a->n || d < 0 || d >= a->n) {
    printf("Estado fora do intervalo em: %d %c %d\n", o, sim, d);
    return 0;
  }
  if (s < 0) {
    printf("Simbolo '%c' nao pertence ao alfabeto.\n", sim);
    return 0;
  }
  a->trans[o][s] |= (1ULL << d);
  return 1;
}

static void inicializarAFN(AFN *a) {
  a->n = 0;
  a->m = 0;
  a->inicial = 0;
  a->finais_mask = 0;
  for (int i = 0; i < MAX_ESTADOS; i++) {
    a->final[i] = 0;
    for (int j = 0; j < MAX_SIMBOLOS; j++) {
      a->trans[i][j] = 0;
    }
  }
}

int lerAFN(AFN *a) {
  inicializarAFN(a);
  int nf, fin;

  printf("Quantidade de estados (estados serao 0..n-1): ");
  if (scanf("%d", &a->n) != 1 || a->n < 1 || a->n > MAX_AFN_ESTADOS) {
    printf("Quantidade de estados invalida (1 a %d).\n", MAX_AFN_ESTADOS);
    return 0;
  }

  printf("Tamanho do alfabeto: ");
  if (scanf("%d", &a->m) != 1 || a->m < 1 || a->m > MAX_SIMBOLOS) {
    printf("Tamanho de alfabeto invalido (1 a %d).\n", MAX_SIMBOLOS);
    return 0;
  }

  printf("Simbolos (separados por espaco): ");
  for (int s = 0; s < a->m; s++) {
    if (scanf(" %c", &a->alfabeto[s]) != 1) {
      printf("Erro ao ler o alfabeto.\n");
      return 0;
    }
  }

  printf("Estado inicial: ");
  if (scanf("%d", &a->inicial) != 1 || a->inicial < 0 || a->inicial >= a->n) {
    printf("Estado inicial invalido.\n");
    return 0;
  }

  printf("Quantidade de estados finais: ");
  if (scanf("%d", &nf) != 1 || nf < 0 || nf > a->n) {
    printf("Quantidade de finais invalida.\n");
    return 0;
  }
  if (nf > 0)
    printf("Estados finais: ");
  for (int i = 0; i < nf; i++) {
    if (scanf("%d", &fin) != 1 || fin < 0 || fin >= a->n) {
      printf("Estado final invalido.\n");
      return 0;
    }
    a->final[fin] = 1;
    a->finais_mask |= (1ULL << fin);
  }

  int qtd;
  printf("Quantidade de transicoes: ");
  if (scanf("%d", &qtd) != 1 || qtd < 0) {
    printf("Quantidade de transicoes invalida.\n");
    return 0;
  }
  if (qtd > 0)
    printf("Digite cada transicao como: origem simbolo destino (ex: 0 a 1)\n");
  for (int t = 0; t < qtd; t++) {
    int o, d;
    char sim;
    printf("Transicao %d: ", t + 1);
    if (scanf("%d %c %d", &o, &sim, &d) != 3 || !adicionarTransicao(a, o, sim, d))
      return 0;
  }
  return 1;
}

static int lerLinhaUtil(FILE *f, char *buf, int max) {
  while (fgets(buf, max, f) != NULL) {
    buf[strcspn(buf, "\r\n")] = '\0';
    char *p = buf;
    while (*p && isspace((unsigned char)*p))
      p++;
    if (*p == '\0' || *p == '#' || (*p == '/' && *(p + 1) == '/'))
      continue;
    if (p != buf)
      memmove(buf, p, strlen(p) + 1);
    return 1;
  }
  return 0;
}

int lerAFNArquivo(const char *nome, AFN *a) {
  FILE *f = fopen(nome, "r");
  if (!f) {
    printf("Nao foi possivel abrir o arquivo '%s'.\n", nome);
    return 0;
  }

  inicializarAFN(a);
  char buf[512];

  /* 1) Quantidade de estados e tamanho do alfabeto */
  if (!lerLinhaUtil(f, buf, sizeof(buf))) {
    fclose(f);
    return 0;
  }
  int n_val = 0, m_val = 0;
  int lidos = sscanf(buf, "%d %d", &n_val, &m_val);
  if (lidos < 1 || n_val < 1 || n_val > MAX_AFN_ESTADOS) {
    fclose(f);
    return 0;
  }
  a->n = n_val;
  if (lidos == 2 && m_val >= 1 && m_val <= MAX_SIMBOLOS)
    a->m = m_val;

  /* 2) Simbolos do alfabeto */
  if (!lerLinhaUtil(f, buf, sizeof(buf))) {
    fclose(f);
    return 0;
  }
  char *tok = strtok(buf, " \t,");
  int count = 0;
  while (tok && count < MAX_SIMBOLOS) {
    a->alfabeto[count++] = tok[0];
    tok = strtok(NULL, " \t,");
  }
  if (a->m == 0)
    a->m = count;

  /* 3) Estado inicial */
  if (!lerLinhaUtil(f, buf, sizeof(buf)) || sscanf(buf, "%d", &a->inicial) != 1 ||
      a->inicial < 0 || a->inicial >= a->n) {
    fclose(f);
    return 0;
  }

  /* 4) Finais: qtd_finais f1 f2 ... */
  if (!lerLinhaUtil(f, buf, sizeof(buf))) {
    fclose(f);
    return 0;
  }
  int nf = 0;
  tok = strtok(buf, " \t,");
  if (tok) {
    nf = atoi(tok);
    for (int i = 0; i < nf; i++) {
      tok = strtok(NULL, " \t,");
      if (!tok)
        break;
      int fin = atoi(tok);
      if (fin >= 0 && fin < a->n) {
        a->final[fin] = 1;
        a->finais_mask |= (1ULL << fin);
      }
    }
  }

  /* 5) Transicoes ate EOF */
  while (lerLinhaUtil(f, buf, sizeof(buf))) {
    tok = strtok(buf, " \t,");
    if (!tok)
      continue;
    int o = atoi(tok);

    tok = strtok(NULL, " \t,");
    if (!tok)
      continue;
    char sim = tok[0];

    while ((tok = strtok(NULL, " \t,")) != NULL) {
      if (strcmp(tok, "-") == 0)
        continue;
      adicionarTransicao(a, o, sim, atoi(tok));
    }
  }

  fclose(f);
  return 1;
}

static void imprimirSubconjunto(uint64_t mask, int n) {
  if (mask == 0) {
    printf("{ }");
    return;
  }
  printf("{");
  int prim = 1;
  for (int e = 0; e < n; e++) {
    if (mask & (1ULL << e)) {
      if (!prim)
        printf(",");
      printf("q%d", e);
      prim = 0;
    }
  }
  printf("}");
}

void imprimirAFN(const AFN *a) {
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
  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      printf("  delta(q%d, %c) = ", i, a->alfabeto[s]);
      imprimirSubconjunto(a->trans[i][s], a->n);
      printf("\n");
    }
  }
}

int converterAFNparaAFD(const AFN *afn, AFD *dfa) {
  dfa->n = 0;
  dfa->m = afn->m;
  dfa->inicial = 0;
  for (int s = 0; s < afn->m; s++)
    dfa->alfabeto[s] = afn->alfabeto[s];

  for (int i = 0; i < MAX_ESTADOS; i++) {
    dfa->final[i] = 0;
    for (int s = 0; s < afn->m; s++)
      dfa->delta[i][s] = -1;
  }

  uint64_t sub[MAX_ESTADOS];
  uint64_t mask_ini = (1ULL << afn->inicial);

  sub[0] = mask_ini;
  dfa->n = 1;
  dfa->final[0] = (mask_ini & afn->finais_mask) ? 1 : 0;

  int fila[MAX_ESTADOS];
  int ini = 0, fim = 0;
  fila[fim++] = 0;

  while (ini < fim) {
    int u = fila[ini++];
    uint64_t mask_u = sub[u];

    for (int j = 0; j < afn->m; j++) {
      uint64_t dest = 0;
      for (int e = 0; e < afn->n; e++) {
        if (mask_u & (1ULL << e))
          dest |= afn->trans[e][j];
      }

      if (dest == 0)
        continue;

      int idx = -1;
      for (int k = 0; k < dfa->n; k++) {
        if (sub[k] == dest) {
          idx = k;
          break;
        }
      }

      if (idx == -1) {
        if (dfa->n >= MAX_ESTADOS)
          return 0;
        idx = dfa->n++;
        sub[idx] = dest;
        dfa->final[idx] = (dest & afn->finais_mask) ? 1 : 0;
        fila[fim++] = idx;
      }
      dfa->delta[u][j] = idx;
    }
  }
  return 1;
}

void menuConversorAFNparaAFD(void) {
  AFN afn;
  AFD dfa;

  limparTela();
  imprimirCabecalho("CONVERSOR AFN PARA AFD");

  int opcao, ok;
  char nome[256];

  printf("Como deseja informar o AFN?\n");
  printf("1 - Digitar\n");
  printf("2 - Ler de arquivo .txt\n");
  printf("Opcao: ");
  if (scanf("%d", &opcao) != 1)
    opcao = 1;

  if (opcao == 2) {
    printf("Nome do arquivo: ");
    scanf("%255s", nome);
    ok = lerAFNArquivo(nome, &afn);
  } else {
    printf("\n");
    ok = lerAFN(&afn);
  }

  if (!ok) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  if (!converterAFNparaAFD(&afn, &dfa)) {
    printf("Nao foi possivel converter (limite de estados do AFD excedido).\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  printf("\n--- AFN original ---\n");
  imprimirAFN(&afn);
  printf("\n--- AFD convertido ---\n");
  imprimirAFD(&dfa);

  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF)
    ;
  pausarTela();
}