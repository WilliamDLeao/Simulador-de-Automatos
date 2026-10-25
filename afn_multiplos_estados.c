#include "afn_multiplos_estados.h"
#include "commons.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int obterIndiceSimboloMultiplo(const AFNMultiplo *a, char c) {
  for (int i = 0; i < a->m; i++) {
    if (a->alfabeto[i] == c)
      return i;
  }
  return -1;
}

static int adicionarTransicaoMultiplo(AFNMultiplo *a, int o, char sim, int d) {
  if (o < 0 || o >= a->n || d < 0 || d >= a->n) {
    printf("Estado fora do intervalo: %d %c %d (intervalo: 0 a %d)\n", o, sim, d, a->n - 1);
    return 0;
  }

  /* Se o simbolo for lambda ('l' ou 'L') */
  if (sim == SIMBOLO_LAMBDA || sim == 'L') {
    a->trans_lambda[o] |= (1ULL << d);
    return 1;
  }

  int s = obterIndiceSimboloMultiplo(a, sim);
  if (s < 0) {
    printf("Simbolo '%c' nao pertence ao alfabeto.\n", sim);
    return 0;
  }

  a->trans[o][s] |= (1ULL << d);
  return 1;
}

static void inicializarAFNMultiplo(AFNMultiplo *a) {
  a->n = 0;
  a->m = 0;
  a->qtd_iniciais = 0;
  a->iniciais_mask = 0;
  a->qtd_finais = 0;
  a->finais_mask = 0;
  for (int i = 0; i < MAX_ESTADOS; i++) {
    a->inicial[i] = 0;
    a->final[i] = 0;
    a->trans_lambda[i] = 0;
    for (int j = 0; j < MAX_SIMBOLOS; j++) {
      a->trans[i][j] = 0;
    }
  }
}

static void inicializarAFNLambda(AFNLambda *a) {
  a->n = 0;
  a->m = 0;
  a->inicial = 0;
  a->qtd_finais = 0;
  a->finais_mask = 0;
  for (int i = 0; i < MAX_ESTADOS; i++) {
    a->final[i] = 0;
    a->trans_lambda[i] = 0;
    for (int j = 0; j < MAX_SIMBOLOS; j++) {
      a->trans[i][j] = 0;
    }
  }
}

/* Leitura de linha com descarte de linhas vazias e comentarios (# ou //) */
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

int lerAFNMultiplo(AFNMultiplo *a) {
  inicializarAFNMultiplo(a);

  printf("Quantidade de estados (1 a %d): ", MAX_AFN_ME_ESTADOS - 2);
  if (scanf("%d", &a->n) != 1 || a->n < 1 || a->n > MAX_AFN_ME_ESTADOS - 2) {
    printf("Quantidade de estados invalida.\n");
    return 0;
  }

  printf("Tamanho do alfabeto (1 a %d): ", MAX_SIMBOLOS);
  if (scanf("%d", &a->m) != 1 || a->m < 1 || a->m > MAX_SIMBOLOS) {
    printf("Tamanho de alfabeto invalido.\n");
    return 0;
  }

  printf("Simbolos (separados por espaco, 'l' e reservado para lambda): ");
  for (int s = 0; s < a->m; s++) {
    if (scanf(" %c", &a->alfabeto[s]) != 1) {
      printf("Erro ao ler o alfabeto.\n");
      return 0;
    }
    if (a->alfabeto[s] == SIMBOLO_LAMBDA || a->alfabeto[s] == 'L') {
      printf("O simbolo 'l' e reservado para transicoes lambda.\n");
      return 0;
    }
  }

  printf("Quantidade de estados iniciais (1 a %d): ", a->n);
  if (scanf("%d", &a->qtd_iniciais) != 1 || a->qtd_iniciais < 1 || a->qtd_iniciais > a->n) {
    printf("Quantidade de estados iniciais invalida.\n");
    return 0;
  }

  printf("Informe os %d estados iniciais (numeros de 0 a %d): ", a->qtd_iniciais, a->n - 1);
  for (int i = 0; i < a->qtd_iniciais; i++) {
    int ini;
    if (scanf("%d", &ini) != 1 || ini < 0 || ini >= a->n) {
      printf("Estado inicial invalido: %d\n", ini);
      return 0;
    }
    if (a->inicial[ini]) {
      printf("Estado inicial repetido: q%d\n", ini);
      i--;
      continue;
    }
    a->inicial[ini] = 1;
    a->iniciais_mask |= (1ULL << ini);
  }

  printf("Quantidade de estados finais (0 a %d): ", a->n);
  if (scanf("%d", &a->qtd_finais) != 1 || a->qtd_finais < 0 || a->qtd_finais > a->n) {
    printf("Quantidade de estados finais invalida.\n");
    return 0;
  }

  if (a->qtd_finais > 0) {
    printf("Informe os %d estados finais (numeros de 0 a %d): ", a->qtd_finais, a->n - 1);
    for (int i = 0; i < a->qtd_finais; i++) {
      int fin;
      if (scanf("%d", &fin) != 1 || fin < 0 || fin >= a->n) {
        printf("Estado final invalido: %d\n", fin);
        return 0;
      }
      if (a->final[fin]) {
        printf("Estado final repetido: q%d\n", fin);
        i--;
        continue;
      }
      a->final[fin] = 1;
      a->finais_mask |= (1ULL << fin);
    }
  }

  int qtd;
  printf("Quantidade de transicoes: ");
  if (scanf("%d", &qtd) != 1 || qtd < 0) {
    printf("Quantidade de transicoes invalida.\n");
    return 0;
  }

  if (qtd > 0)
    printf("Digite cada transicao como: origem simbolo destino (ex: 0 a 1 ou 0 l 2)\n");

  for (int t = 0; t < qtd; t++) {
    int o, d;
    char sim;
    printf("Transicao %d: ", t + 1);
    if (scanf("%d %c %d", &o, &sim, &d) != 3 || !adicionarTransicaoMultiplo(a, o, sim, d))
      return 0;
  }

  return 1;
}

int lerAFNMultiploArquivo(const char *nome, AFNMultiplo *a) {
  FILE *f = fopen(nome, "r");
  if (!f) {
    printf("Nao foi possivel abrir o arquivo '%s'.\n", nome);
    return 0;
  }

  inicializarAFNMultiplo(a);
  char buf[512];

  /* 1) Quantidade de estados e tamanho do alfabeto */
  if (!lerLinhaUtil(f, buf, sizeof(buf))) {
    fclose(f);
    return 0;
  }
  int n_val = 0, m_val = 0;
  int lidos = sscanf(buf, "%d %d", &n_val, &m_val);
  if (lidos < 1 || n_val < 1 || n_val > MAX_AFN_ME_ESTADOS - 2) {
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

  /* 3) Estados iniciais: <qtd_iniciais> <i1> <i2> ... (ou se for 1 unico numero no arquivo) */
  if (!lerLinhaUtil(f, buf, sizeof(buf))) {
    fclose(f);
    return 0;
  }

  tok = strtok(buf, " \t,");
  if (!tok) {
    fclose(f);
    return 0;
  }

  int primeiro_num = atoi(tok);
  char *prox_tok = strtok(NULL, " \t,");

  if (!prox_tok) {
    /* Formato antigo com apenas 1 estado inicial direto: ex '0' */
    if (primeiro_num >= 0 && primeiro_num < a->n) {
      a->qtd_iniciais = 1;
      a->inicial[primeiro_num] = 1;
      a->iniciais_mask |= (1ULL << primeiro_num);
    } else {
      fclose(f);
      return 0;
    }
  } else {
    /* Formato <qtd_iniciais> <i1> <i2> ... */
    a->qtd_iniciais = primeiro_num;
    for (int i = 0; i < a->qtd_iniciais && prox_tok; i++) {
      int ini = atoi(prox_tok);
      if (ini >= 0 && ini < a->n) {
        a->inicial[ini] = 1;
        a->iniciais_mask |= (1ULL << ini);
      }
      prox_tok = strtok(NULL, " \t,");
    }
  }

  /* 4) Finais: <qtd_finais> <f1> <f2> ... */
  if (!lerLinhaUtil(f, buf, sizeof(buf))) {
    fclose(f);
    return 0;
  }
  tok = strtok(buf, " \t,");
  if (tok) {
    a->qtd_finais = atoi(tok);
    for (int i = 0; i < a->qtd_finais; i++) {
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

  /* 5) Transicoes ate EOF (suporta linha opcional com quantidade de transicoes) */
  int primeira_linha_trans = 1;
  while (lerLinhaUtil(f, buf, sizeof(buf))) {
    tok = strtok(buf, " \t,");
    if (!tok)
      continue;

    char *segundo = strtok(NULL, " \t,");
    if (primeira_linha_trans && !segundo) {
      /* Se a primeira linha tiver apenas 1 token, e a quantidade de transicoes */
      primeira_linha_trans = 0;
      continue;
    }
    primeira_linha_trans = 0;

    int o = atoi(tok);
    if (!segundo)
      continue;
    char sim = segundo[0];

    while ((tok = strtok(NULL, " \t,")) != NULL) {
      if (strcmp(tok, "-") == 0)
        continue;
      adicionarTransicaoMultiplo(a, o, sim, atoi(tok));
    }
  }

  fclose(f);
  return 1;
}

static void imprimirSubconjuntoBits(uint64_t mask, int n) {
  if (mask == 0) {
    printf("{ }");
    return;
  }
  printf("{");
  int prim = 1;
  for (int e = 0; e < n; e++) {
    if (mask & (1ULL << e)) {
      if (!prim)
        printf(", ");
      printf("q%d", e);
      prim = 0;
    }
  }
  printf("}");
}

void imprimirAFNMultiplo(const AFNMultiplo *a) {
  printf("Quantidade de estados: %d (q0 .. q%d)\n", a->n, a->n - 1);
  printf("Alfabeto            : ");
  for (int s = 0; s < a->m; s++)
    printf("%c ", a->alfabeto[s]);
  printf("\nEstados iniciais    : ");
  imprimirSubconjuntoBits(a->iniciais_mask, a->n);
  printf(" (%d %s)\n", a->qtd_iniciais, a->qtd_iniciais > 1 ? "iniciais" : "inicial");

  printf("Estados finais      : ");
  imprimirSubconjuntoBits(a->finais_mask, a->n);
  printf("\nTransicoes:\n");

  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      printf("  delta(q%d, %c) = ", i, a->alfabeto[s]);
      imprimirSubconjuntoBits(a->trans[i][s], a->n);
      printf("\n");
    }
    if (a->trans_lambda[i] != 0) {
      printf("  delta(q%d, lambda) = ", i);
      imprimirSubconjuntoBits(a->trans_lambda[i], a->n);
      printf("\n");
    }
  }
}

void imprimirAFNLambda(const AFNLambda *a) {
  printf("Quantidade de estados: %d (q0 .. q%d)\n", a->n, a->n - 1);
  printf("Alfabeto            : ");
  for (int s = 0; s < a->m; s++)
    printf("%c ", a->alfabeto[s]);
  printf("\nEstado inicial unico: q%d\n", a->inicial);
  printf("Estados finais      : ");
  imprimirSubconjuntoBits(a->finais_mask, a->n);
  printf("\nTransicoes:\n");

  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      printf("  delta(q%d, %c) = ", i, a->alfabeto[s]);
      imprimirSubconjuntoBits(a->trans[i][s], a->n);
      printf("\n");
    }
    printf("  delta(q%d, lambda) = ", i);
    imprimirSubconjuntoBits(a->trans_lambda[i], a->n);
    printf("\n");
  }
}

int converterMultiplosParaLambda(const AFNMultiplo *entrada, AFNLambda *saida, int unificar_finais) {
  int estados_necessarios = entrada->n + 1 + (unificar_finais ? 1 : 0);
  if (estados_necessarios > MAX_ESTADOS) {
    printf("Limite maximo de estados excedido (%d > %d).\n", estados_necessarios, MAX_ESTADOS);
    return 0;
  }

  inicializarAFNLambda(saida);
  saida->n = entrada->n;
  saida->m = entrada->m;
  for (int s = 0; s < entrada->m; s++)
    saida->alfabeto[s] = entrada->alfabeto[s];

  /* 1) Copia todas as transicoes originais por simbolo e lambda */
  for (int i = 0; i < entrada->n; i++) {
    saida->trans_lambda[i] = entrada->trans_lambda[i];
    for (int s = 0; s < entrada->m; s++) {
      saida->trans[i][s] = entrada->trans[i][s];
    }
  }

  /* 2) Copia estados finais */
  saida->qtd_finais = entrada->qtd_finais;
  saida->finais_mask = entrada->finais_mask;
  for (int i = 0; i < entrada->n; i++) {
    saida->final[i] = entrada->final[i];
  }

  /* 3) Cria um NOVO estado inicial unico q_novo_ini */
  int novo_ini = saida->n++;
  saida->inicial = novo_ini;
  saida->final[novo_ini] = 0;
  saida->trans_lambda[novo_ini] = 0;
  for (int s = 0; s < saida->m; s++)
    saida->trans[novo_ini][s] = 0;

  /* Adiciona transicoes lambda do novo inicial para todos os multiplos iniciais originais */
  for (int i = 0; i < entrada->n; i++) {
    if (entrada->inicial[i]) {
      saida->trans_lambda[novo_ini] |= (1ULL << i);
    }
  }

  /* 4) Se solicitado, unifica tambem os multiplos estados finais em um novo unico final */
  if (unificar_finais && entrada->qtd_finais > 0) {
    int novo_fim = saida->n++;
    saida->trans_lambda[novo_fim] = 0;
    for (int s = 0; s < saida->m; s++)
      saida->trans[novo_fim][s] = 0;

    /* Todos os antigos estados finais passam a apontar para o novo_fim via lambda */
    for (int i = 0; i < entrada->n; i++) {
      if (saida->final[i]) {
        saida->trans_lambda[i] |= (1ULL << novo_fim);
        saida->final[i] = 0;
      }
    }
    saida->final[novo_fim] = 1;
    saida->finais_mask = (1ULL << novo_fim);
    saida->qtd_finais = 1;
  }

  return 1;
}

int salvarAFNLambdaArquivo(const char *nome, const AFNLambda *a) {
  FILE *f = fopen(nome, "w");
  if (!f) {
    printf("Erro ao criar arquivo '%s' para salvar.\n", nome);
    return 0;
  }

  /* 1) Quantidade de estados e tamanho do alfabeto */
  fprintf(f, "%d %d\n", a->n, a->m);

  /* 2) Alfabeto */
  for (int s = 0; s < a->m; s++) {
    fprintf(f, "%c%s", a->alfabeto[s], s == a->m - 1 ? "" : " ");
  }
  fprintf(f, "\n");

  /* 3) Estado inicial unico */
  fprintf(f, "%d\n", a->inicial);

  /* 4) Estados finais */
  fprintf(f, "%d", a->qtd_finais);
  for (int i = 0; i < a->n; i++) {
    if (a->final[i])
      fprintf(f, " %d", i);
  }
  fprintf(f, "\n");

  /* Conta total de transicoes */
  int total_trans = 0;
  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      for (int d = 0; d < a->n; d++) {
        if (a->trans[i][s] & (1ULL << d))
          total_trans++;
      }
    }
    for (int d = 0; d < a->n; d++) {
      if (a->trans_lambda[i] & (1ULL << d))
        total_trans++;
    }
  }

  /* 5) Quantidade de transicoes */
  fprintf(f, "%d\n", total_trans);

  /* 6) Transicoes: origem simbolo destino */
  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      for (int d = 0; d < a->n; d++) {
        if (a->trans[i][s] & (1ULL << d))
          fprintf(f, "%d %c %d\n", i, a->alfabeto[s], d);
      }
    }
    for (int d = 0; d < a->n; d++) {
      if (a->trans_lambda[i] & (1ULL << d))
        fprintf(f, "%d %c %d\n", i, SIMBOLO_LAMBDA, d);
    }
  }

  fclose(f);
  return 1;
}

void menuAFNMultiplosParaLambda(void) {
  AFNMultiplo entrada;
  AFNLambda convertida;

  limparTela();
  imprimirCabecalho("CONVERTER AFN COM MULTIPLOS ESTADOS PARA AFN-LAMBDA");

  int opcao, ok;
  char nome[256];

  printf("Como deseja informar o AFN com multiplos estados?\n");
  printf("1 - Digitar pelo teclado\n");
  printf("2 - Ler de arquivo .txt\n");
  printf("Opcao: ");
  if (scanf("%d", &opcao) != 1)
    opcao = 1;

  if (opcao == 2) {
    printf("Nome do arquivo (ex: exemplo_afn_multiplos.txt): ");
    scanf("%255s", nome);
    ok = lerAFNMultiploArquivo(nome, &entrada);
  } else {
    printf("\n");
    ok = lerAFNMultiplo(&entrada);
  }

  if (!ok) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  printf("\nModo de unificacao:\n");
  printf("1 - Unificar apenas estados iniciais (gera novo q_ini -> iniciais via lambda)\n");
  printf("2 - Unificar estados iniciais E estados finais (gera novo q_ini e novo q_fim)\n");
  printf("Opcao: ");
  int modo = 1;
  if (scanf("%d", &modo) != 1)
    modo = 1;

  int unificar_finais = (modo == 2);

  if (!converterMultiplosParaLambda(&entrada, &convertida, unificar_finais)) {
    printf("\nNao foi possivel converter (limite de estados excedido).\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  printf("\n--- AFN ORIGINAL COM MULTIPLOS ESTADOS ---\n");
  imprimirAFNMultiplo(&entrada);

  printf("\n--- AFN-LAMBDA RESULTANTE (UNIFICADO) ---\n");
  imprimirAFNLambda(&convertida);

  printf("\nDeseja salvar o AFN-lambda resultante em um arquivo .txt?\n");
  printf("1 - Sim\n");
  printf("2 - Nao\n");
  printf("Opcao: ");
  int op_salvar = 2;
  if (scanf("%d", &op_salvar) != 1)
    op_salvar = 2;

  if (op_salvar == 1) {
    char nome_salvar[256];
    printf("Nome do arquivo para salvar (ex: afn_lambda_gerado.txt): ");
    scanf("%255s", nome_salvar);
    if (salvarAFNLambdaArquivo(nome_salvar, &convertida)) {
      printf("Arquivo '%s' salvo com sucesso! Compativel com a opcao de conversao AFN-lambda.\n", nome_salvar);
    }
  }

  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF)
    ;
  pausarTela();
}
