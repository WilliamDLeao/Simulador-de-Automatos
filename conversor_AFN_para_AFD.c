#include "conversor_AFN_para_AFD.h"
#include "commons.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Armazena os subconjuntos da ultima conversao para uso em imprimir_afd e simular_cadeia_afd */
static uint64_t ultimos_subconjuntos[MAX_ESTADOS];
static bool tem_subconjuntos_salvos = false;

void inicializar_afn(AFN *afn) {
  if (!afn)
    return;
  afn->n = 0;
  afn->m = 0;
  afn->inicial = 0;
  afn->finais_mask = 0;
  for (int i = 0; i < MAX_SIMBOLOS; i++) {
    afn->alfabeto[i] = '\0';
  }
  for (int i = 0; i < MAX_ESTADOS; i++) {
    afn->final[i] = 0;
    for (int j = 0; j < MAX_SIMBOLOS; j++) {
      afn->trans[i][j] = 0; /* 0 significa nenhum estado de destino */
    }
  }
}

void inicializar_afd(AFD *dfa) {
  if (!dfa)
    return;
  dfa->n = 0;
  dfa->m = 0;
  dfa->inicial = 0;
  for (int i = 0; i < MAX_SIMBOLOS; i++) {
    dfa->alfabeto[i] = '\0';
  }
  for (int i = 0; i < MAX_ESTADOS; i++) {
    dfa->final[i] = 0;
    for (int j = 0; j < MAX_SIMBOLOS; j++) {
      dfa->delta[i][j] = -1; /* -1 significa sem transicao */
    }
  }
}

int obter_indice_simbolo(const AFN *afn, char c) {
  if (!afn)
    return -1;
  for (int i = 0; i < afn->m; i++) {
    if (afn->alfabeto[i] == c) {
      return i;
    }
  }
  return -1;
}

int obter_indice_simbolo_afd(const AFD *dfa, char c) {
  if (!dfa)
    return -1;
  for (int i = 0; i < dfa->m; i++) {
    if (dfa->alfabeto[i] == c) {
      return i;
    }
  }
  return -1;
}

int adicionarTransicaoAFN(AFN *a, int o, char sim, int d) {
  if (!a)
    return 0;
  int s = obter_indice_simbolo(a, sim);

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

/* Leitura generica do teclado para AFN (no mesmo padrao de lerAFD em afd.c) */
int lerAFN(AFN *a) {
  inicializar_afn(a);
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
  if (qtd > 0) {
    printf("Digite cada transicao como: origem simbolo destino (ex: 0 a 1)\n");
  }
  for (int t = 0; t < qtd; t++) {
    int o, d;
    char sim;
    printf("Transicao %d: ", t + 1);
    if (scanf("%d %c %d", &o, &sim, &d) != 3 || !adicionarTransicaoAFN(a, o, sim, d)) {
      return 0;
    }
  }
  return 1;
}

/* Le a proxima linha util do arquivo (ignora vazias e comentarios iniciados por # ou //) */
static bool ler_proxima_linha_util(FILE *f, char *buffer, int max_tam) {
  while (fgets(buffer, max_tam, f) != NULL) {
    buffer[strcspn(buffer, "\r\n")] = '\0';
    char *ptr = buffer;
    while (*ptr && isspace((unsigned char)*ptr))
      ptr++;
    if (*ptr == '\0' || *ptr == '#' || (*ptr == '/' && *(ptr + 1) == '/')) {
      continue;
    }
    if (ptr != buffer) {
      memmove(buffer, ptr, strlen(ptr) + 1);
    }
    return true;
  }
  return false;
}

/* Leitura de AFN de arquivo .txt no formato padrao do projeto:
   Linha 1: <n> <m>
   Linha 2: <s1> <s2> ... <sm>
   Linha 3: <inicial>
   Linha 4: <nf> <f1> <f2> ... <fnf>
   Linhas seguintes: transicoes (origem simbolo destino...) ate EOF
*/
int lerAFNArquivo(const char *nome, AFN *a) {
  FILE *f = fopen(nome, "r");
  if (!f) {
    printf("\nNao foi possivel abrir o arquivo '%s'.\n", nome);
    return 0;
  }

  inicializar_afn(a);
  char buffer[512];

  /* 1) Quantidade de estados e tamanho do alfabeto */
  if (!ler_proxima_linha_util(f, buffer, sizeof(buffer))) {
    printf("Arquivo vazio ou formato invalido ao ler estados/alfabeto.\n");
    fclose(f);
    return 0;
  }
  int n_val = 0, m_val = 0;
  int lidos = sscanf(buffer, "%d %d", &n_val, &m_val);
  if (lidos < 1 || n_val < 1 || n_val > MAX_AFN_ESTADOS) {
    printf("Quantidade de estados invalida no arquivo: %d (max: %d).\n", n_val, MAX_AFN_ESTADOS);
    fclose(f);
    return 0;
  }
  a->n = n_val;
  if (lidos == 2) {
    if (m_val < 1 || m_val > MAX_SIMBOLOS) {
      printf("Tamanho de alfabeto invalido no arquivo: %d.\n", m_val);
      fclose(f);
      return 0;
    }
    a->m = m_val;
  }

  /* 2) Simbolos do alfabeto */
  if (!ler_proxima_linha_util(f, buffer, sizeof(buffer))) {
    printf("Fim de arquivo inesperado ao ler simbolos do alfabeto.\n");
    fclose(f);
    return 0;
  }
  char *tok = strtok(buffer, " \t,");
  int count_simb = 0;
  while (tok != NULL && count_simb < MAX_SIMBOLOS) {
    a->alfabeto[count_simb++] = tok[0];
    tok = strtok(NULL, " \t,");
  }
  if (a->m == 0) {
    a->m = count_simb;
  }
  if (a->m < 1) {
    printf("Nenhum simbolo de alfabeto valido encontrado.\n");
    fclose(f);
    return 0;
  }

  /* 3) Estado inicial */
  if (!ler_proxima_linha_util(f, buffer, sizeof(buffer))) {
    printf("Fim de arquivo inesperado ao ler estado inicial.\n");
    fclose(f);
    return 0;
  }
  if (sscanf(buffer, "%d", &a->inicial) != 1 || a->inicial < 0 || a->inicial >= a->n) {
    printf("Estado inicial invalido no arquivo: %d.\n", a->inicial);
    fclose(f);
    return 0;
  }

  /* 4) Quantidade de finais e estados finais */
  if (!ler_proxima_linha_util(f, buffer, sizeof(buffer))) {
    printf("Fim de arquivo inesperado ao ler estados finais.\n");
    fclose(f);
    return 0;
  }
  int nums_finais[MAX_ESTADOS];
  int total_finais_linha = 0;
  tok = strtok(buffer, " \t,");
  while (tok != NULL && total_finais_linha < MAX_ESTADOS) {
    if (isdigit((unsigned char)tok[0]) || (tok[0] == '-' && isdigit((unsigned char)tok[1]))) {
      nums_finais[total_finais_linha++] = atoi(tok);
    }
    tok = strtok(NULL, " \t,");
  }

  if (total_finais_linha > 0) {
    int nf = nums_finais[0];
    /* Se a linha tem a contagem nf seguida dos nf estados (ex: "1 3" ou "2 1 2") */
    if (nf >= 0 && nf <= a->n && nf == total_finais_linha - 1) {
      for (int i = 1; i <= nf; i++) {
        int fin = nums_finais[i];
        if (fin >= 0 && fin < a->n) {
          a->final[fin] = 1;
          a->finais_mask |= (1ULL << fin);
        }
      }
    } else {
      /* Caso a linha contenha diretamente os IDs dos finais (ex: "3" ou "1 2") */
      for (int i = 0; i < total_finais_linha; i++) {
        int fin = nums_finais[i];
        if (fin >= 0 && fin < a->n) {
          a->final[fin] = 1;
          a->finais_mask |= (1ULL << fin);
        }
      }
    }
  }

  /* 5) Transicoes ate EOF */
  int count_trans = 0;
  while (ler_proxima_linha_util(f, buffer, sizeof(buffer))) {
    tok = strtok(buffer, " \t,");
    if (!tok)
      continue;
    int o = atoi(tok);

    tok = strtok(NULL, " \t,");
    if (!tok)
      continue;
    char sim = tok[0];

    int idx_sim = obter_indice_simbolo(a, sim);
    if (idx_sim < 0) {
      printf("Aviso: Simbolo '%c' na transicao nao pertence ao alfabeto. Ignorando.\n", sim);
      continue;
    }

    /* Le um ou mais destinos na mesma linha (ex: "0 a 1" ou "0 a 1 2") */
    while ((tok = strtok(NULL, " \t,")) != NULL) {
      if (strcmp(tok, "-") == 0)
        continue; /* '-' representa conjunto vazio */
      int d = atoi(tok);
      if (o >= 0 && o < a->n && d >= 0 && d < a->n) {
        a->trans[o][idx_sim] |= (1ULL << d);
        count_trans++;
      }
    }
  }

  fclose(f);
  return 1;
}

int carregar_afn_arquivo(const char *caminho, AFN *a) {
  return lerAFNArquivo(caminho, a);
}

/* Converte AFN para AFD usando algoritmo de Construcao de Subconjuntos (BFS) */
int converter_afn_para_afd_subconjuntos(const AFN *afn, AFD *dfa, uint64_t *subconjuntos) {
  if (!afn || !dfa)
    return 0;

  inicializar_afd(dfa);
  dfa->m = afn->m;
  dfa->inicial = 0;
  for (int s = 0; s < afn->m; s++) {
    dfa->alfabeto[s] = afn->alfabeto[s];
  }

  if (afn->n <= 0)
    return 0;

  uint64_t subconj_local[MAX_ESTADOS];
  uint64_t *sub = subconjuntos ? subconjuntos : subconj_local;

  /* Estado inicial do AFD: conjunto contendo o estado inicial do AFN */
  int ini = (afn->inicial >= 0 && afn->inicial < afn->n) ? afn->inicial : 0;
  uint64_t mask_ini = (1ULL << ini);

  sub[0] = mask_ini;
  dfa->n = 1;
  dfa->final[0] = ((mask_ini & afn->finais_mask) != 0) ? 1 : 0;

  int fila[MAX_ESTADOS];
  int fila_ini = 0, fila_fim = 0;
  fila[fila_fim++] = 0;

  while (fila_ini < fila_fim) {
    int u = fila[fila_ini++];
    uint64_t mask_u = sub[u];

    for (int j = 0; j < afn->m; j++) {
      uint64_t dest_mask = 0;

      for (int e = 0; e < afn->n; e++) {
        if (mask_u & (1ULL << e)) {
          dest_mask |= afn->trans[e][j];
        }
      }

      if (dest_mask == 0) {
        dfa->delta[u][j] = -1; /* sem transicao / estado morto */
        continue;
      }

      int idx_existente = -1;
      for (int k = 0; k < dfa->n; k++) {
        if (sub[k] == dest_mask) {
          idx_existente = k;
          break;
        }
      }

      if (idx_existente == -1) {
        if (dfa->n >= MAX_ESTADOS) {
          printf("Limite de estados do AFD (%d) atingido na conversao.\n", MAX_ESTADOS);
          return 0;
        }
        idx_existente = dfa->n++;
        sub[idx_existente] = dest_mask;
        dfa->final[idx_existente] = ((dest_mask & afn->finais_mask) != 0) ? 1 : 0;
        fila[fila_fim++] = idx_existente;
      }

      dfa->delta[u][j] = idx_existente;
    }
  }

  /* Guarda copia dos subconjuntos para impressao posterior */
  for (int i = 0; i < dfa->n; i++) {
    ultimos_subconjuntos[i] = sub[i];
  }
  tem_subconjuntos_salvos = true;

  return 1;
}

int converter_afn_para_afd(AFN *afn, AFD *dfa) {
  return converter_afn_para_afd_subconjuntos(afn, dfa, NULL);
}

void imprimir_subconjunto(uint64_t mask, int max_estados) {
  if (mask == 0) {
    printf("{ }");
    return;
  }
  printf("{");
  bool primeiro = true;
  for (int e = 0; e < max_estados; e++) {
    if (mask & (1ULL << e)) {
      if (!primeiro)
        printf(",");
      printf("q%d", e);
      primeiro = false;
    }
  }
  printf("}");
}

void imprimir_afn(const AFN *afn) {
  if (!afn)
    return;
  imprimirCabecalho("AUTOMATO FINITO NAO-DETERMINISTICO (AFN)");
  printf("Estados   : %d (q0..q%d)\n", afn->n, afn->n - 1);
  printf("Alfabeto  : ");
  for (int s = 0; s < afn->m; s++) {
    printf("%c ", afn->alfabeto[s]);
  }
  printf("\nInicial   : q%d\n", afn->inicial);
  printf("Finais    : ");
  for (int i = 0; i < afn->n; i++) {
    if (afn->final[i]) {
      printf("q%d ", i);
    }
  }
  printf("\nTransicoes:\n");
  for (int i = 0; i < afn->n; i++) {
    for (int s = 0; s < afn->m; s++) {
      uint64_t mask = afn->trans[i][s];
      if (mask == 0) {
        printf("  delta(q%d, %c) = -\n", i, afn->alfabeto[s]);
      } else {
        printf("  delta(q%d, %c) = ", i, afn->alfabeto[s]);
        imprimir_subconjunto(mask, afn->n);
        printf("\n");
      }
    }
  }
}

void imprimir_afd_com_subconjuntos(const AFN *afn, const AFD *dfa, const uint64_t *subconjuntos) {
  if (!dfa)
    return;
  imprimirCabecalho("AUTOMATO FINITO DETERMINISTICO (AFD)");
  printf("Total de estados gerados: %d\n\n", dfa->n);

  if (afn && subconjuntos) {
    printf("Mapeamento de Estados (Subconjuntos do AFN):\n");
    for (int i = 0; i < dfa->n; i++) {
      printf("  S%-2d = ", i);
      imprimir_subconjunto(subconjuntos[i], afn->n);
      if (dfa->final[i]) {
        printf("  [FINAL]");
      }
      printf("\n");
    }
    printf("\n");
  }

  printf("Tabela de Transicoes do AFD:\n");
  printf("  %-10s |", "Estado");
  if (afn && subconjuntos) {
    printf(" %-16s |", "Subconjunto");
  }
  for (int j = 0; j < dfa->m; j++) {
    printf("   '%c'   |", dfa->alfabeto[j]);
  }
  printf(" Final?\n");

  printf("  -----------+");
  if (afn && subconjuntos) {
    printf("------------------+");
  }
  for (int j = 0; j < dfa->m; j++) {
    printf("---------+");
  }
  printf("---------\n");

  for (int i = 0; i < dfa->n; i++) {
    char prefixo[8] = "   ";
    if (i == dfa->inicial && dfa->final[i]) {
      strcpy(prefixo, "->*");
    } else if (i == dfa->inicial) {
      strcpy(prefixo, "-> ");
    } else if (dfa->final[i]) {
      strcpy(prefixo, " * ");
    }

    printf("  %s S%-4d |", prefixo, i);

    if (afn && subconjuntos) {
      char sub_str[64];
      int pos = 0;
      sub_str[pos++] = '{';
      bool p = true;
      for (int e = 0; e < afn->n; e++) {
        if (subconjuntos[i] & (1ULL << e)) {
          if (!p)
            sub_str[pos++] = ',';
          pos += sprintf(sub_str + pos, "q%d", e);
          p = false;
        }
      }
      sub_str[pos++] = '}';
      sub_str[pos] = '\0';
      printf(" %-16s |", sub_str);
    }

    for (int j = 0; j < dfa->m; j++) {
      int dest = dfa->delta[i][j];
      if (dest < 0) {
        printf("    -    |");
      } else {
        printf("   S%-3d |", dest);
      }
    }
    printf("   %s\n", dfa->final[i] ? "SIM" : "NAO");
  }
  printf("  Legenda: '->' Inicial, '*' Final, '-' Sem transicao (estado morto)\n");
}

void imprimir_afd(const AFN *afn, const AFD *dfa) {
  if (tem_subconjuntos_salvos && afn) {
    imprimir_afd_com_subconjuntos(afn, dfa, ultimos_subconjuntos);
  } else {
    imprimirAFD(dfa);
  }
}

bool simular_cadeia_afd_detalhada(const AFN *afn, const AFD *dfa, const uint64_t *subconjuntos,
                                  const char *cadeia) {
  if (!dfa || dfa->n <= 0) {
    printf("\n[ERRO] Nenhum AFD carregado para simulacao!\n");
    return false;
  }

  printf("\n--- Simulando Cadeia \"%s\" no AFD ---\n", cadeia);
  int estado_atual = dfa->inicial;

  printf("  Inicio: S%d ", estado_atual);
  if (afn && subconjuntos) {
    imprimir_subconjunto(subconjuntos[estado_atual], afn->n);
  }
  printf("\n");

  int tam = strlen(cadeia);
  if (tam == 0) {
    printf("  (Cadeia vazia)\n");
  }

  for (int i = 0; i < tam; i++) {
    char c = cadeia[i];
    int idx = obter_indice_simbolo_afd(dfa, c);
    if (idx == -1) {
      printf("  [REJEITADA] Simbolo '%c' nao pertence ao alfabeto!\n", c);
      return false;
    }

    int proximo = dfa->delta[estado_atual][idx];
    if (proximo < 0) {
      printf("  Passo %d: S%d lendo '%c' -> Sem transicao (caiu em erro/estado morto).\n", i + 1,
             estado_atual, c);
      printf("  [REJEITADA] Nao ha transicao a partir de S%d com '%c'.\n", estado_atual, c);
      return false;
    }

    printf("  Passo %d: S%d --('%c')--> S%d ", i + 1, estado_atual, c, proximo);
    if (afn && subconjuntos) {
      imprimir_subconjunto(subconjuntos[proximo], afn->n);
    }
    printf("\n");

    estado_atual = proximo;
  }

  bool aceita = (dfa->final[estado_atual] == 1);
  if (aceita) {
    printf("  Resultado: O estado atingido S%d e de ACEITACAO!\n", estado_atual);
    printf("  >>> [CADEIA ACEITA] <<<\n");
  } else {
    printf("  Resultado: O estado atingido S%d NAO e de aceitacao.\n", estado_atual);
    printf("  >>> [CADEIA REJEITADA] <<<\n");
  }
  return aceita;
}

bool simular_cadeia_afd(const AFN *afn, const AFD *dfa, const char *cadeia) {
  if (tem_subconjuntos_salvos && afn) {
    return simular_cadeia_afd_detalhada(afn, dfa, ultimos_subconjuntos, cadeia);
  }
  return simular_cadeia_afd_detalhada(NULL, dfa, NULL, cadeia);
}

/* Salva o AFD no formato oficial do projeto (100% compativel com lerAFDArquivo) */
int salvarAFDArquivo(const char *caminho_arquivo, const AFD *dfa) {
  if (!dfa)
    return 0;
  FILE *f = fopen(caminho_arquivo, "w");
  if (!f) {
    printf("\n[ERRO] Nao foi possivel criar o arquivo '%s'.\n", caminho_arquivo);
    return 0;
  }

  /* Linha 1: n m */
  fprintf(f, "%d %d\n", dfa->n, dfa->m);

  /* Linha 2: simbolos */
  for (int j = 0; j < dfa->m; j++) {
    fprintf(f, "%c%s", dfa->alfabeto[j], (j < dfa->m - 1) ? " " : "");
  }
  fprintf(f, "\n");

  /* Linha 3: estado inicial */
  fprintf(f, "%d\n", dfa->inicial);

  /* Linha 4: nf seguido dos estados finais */
  int nf = 0;
  for (int i = 0; i < dfa->n; i++) {
    if (dfa->final[i])
      nf++;
  }
  fprintf(f, "%d", nf);
  for (int i = 0; i < dfa->n; i++) {
    if (dfa->final[i]) {
      fprintf(f, " %d", i);
    }
  }
  fprintf(f, "\n");

  /* Linhas seguintes: transicoes origem simbolo destino */
  for (int i = 0; i < dfa->n; i++) {
    for (int j = 0; j < dfa->m; j++) {
      if (dfa->delta[i][j] >= 0) {
        fprintf(f, "%d %c %d\n", i, dfa->alfabeto[j], dfa->delta[i][j]);
      }
    }
  }

  fclose(f);
  printf("\n[SUCESSO] AFD salvo com sucesso em '%s'!\n", caminho_arquivo);
  return 1;
}

int salvar_afd_arquivo(const char *caminho_arquivo, const AFN *afn, const AFD *dfa) {
  (void)afn;
  return salvarAFDArquivo(caminho_arquivo, dfa);
}

/* Menu interativo de conversao AFN -> AFD (compativel com o padrao de menuMinimizarAFD) */
void menuConversorAFNparaAFD(void) {
  AFN afn;
  AFD dfa;
  uint64_t subconjuntos[MAX_ESTADOS];

  limparTela();
  imprimirCabecalho("CONVERSOR AFN PARA AFD");

  int opcao, ok = 0;
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

  if (!converter_afn_para_afd_subconjuntos(&afn, &dfa, subconjuntos)) {
    printf("Nao foi possivel converter (limite de estados do AFD excedido).\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  imprimir_afn(&afn);
  imprimir_afd_com_subconjuntos(&afn, &dfa, subconjuntos);

  /* Menu de operacoes adicionais com o AFD recem-gerado */
  int sub_op = -1;
  do {
    printf("\n--- Opcoes para o AFD Gerado ---\n");
    printf("1 - Testar cadeia (simular)\n");
    printf("2 - Salvar AFD em arquivo .txt\n");
    printf("3 - Exibir tabelas novamente\n");
    printf("0 - Voltar ao menu principal\n");
    printf("Opcao: ");

    if (scanf("%d", &sub_op) != 1) {
      sub_op = -1;
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF)
        ;
      continue;
    }

    /* Limpa newline residual */
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;

    switch (sub_op) {
    case 1: {
      char cadeia[256];
      printf("Digite a palavra/cadeia para testar: ");
      if (fgets(cadeia, sizeof(cadeia), stdin) != NULL) {
        cadeia[strcspn(cadeia, "\r\n")] = '\0';
        simular_cadeia_afd_detalhada(&afn, &dfa, subconjuntos, cadeia);
      }
      break;
    }
    case 2: {
      char arq_saida[256];
      printf("Digite o nome do arquivo para salvar (ex: afd_convertido.txt): ");
      if (fgets(arq_saida, sizeof(arq_saida), stdin) != NULL) {
        arq_saida[strcspn(arq_saida, "\r\n")] = '\0';
      }
      if (strlen(arq_saida) == 0) {
        strcpy(arq_saida, "afd_convertido.txt");
      }
      salvarAFDArquivo(arq_saida, &dfa);
      break;
    }
    case 3:
      limparTela();
      imprimir_afn(&afn);
      imprimir_afd_com_subconjuntos(&afn, &dfa, subconjuntos);
      break;
    case 0:
      break;
    default:
      printf("Opcao invalida!\n");
      break;
    }
  } while (sub_op != 0);
}