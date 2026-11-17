#include "simular_automato.h"
#include "commons.h"
#include "json_reader.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_PALAVRA 512
#define MAX_PALAVRAS 256

typedef struct {
  char itens[MAX_PALAVRAS][MAX_PALAVRA];
  int qtd;
} ListaPalavras;

static void trim(char *s) {
  int len = (int)strlen(s);
  while (len > 0 && isspace((unsigned char)s[len - 1]))
    s[--len] = '\0';
  int inicio = 0;
  while (s[inicio] && isspace((unsigned char)s[inicio]))
    inicio++;
  if (inicio > 0)
    memmove(s, s + inicio, strlen(s + inicio) + 1);
}

static int eh_palavra_parada(const char *s) {
  char b[16];
  size_t i;
  for (i = 0; s[i] && i < sizeof(b) - 1; i++)
    b[i] = (char)tolower((unsigned char)s[i]);
  b[i] = '\0';
  return strcmp(b, "fim") == 0 || strcmp(b, "sair") == 0;
}

static void adicionar_palavra(ListaPalavras *lista, const char *p) {
  if (lista->qtd >= MAX_PALAVRAS || !p)
    return;
  if (p[0] == '=')
    p++;
  else if (strcmp(p, "_") == 0)
    p = "";
  snprintf(lista->itens[lista->qtd], MAX_PALAVRA, "%s", p);
  lista->qtd++;
}

static int ler_palavras_arquivo(const char *caminho, ListaPalavras *lista) {
  FILE *f = fopen(caminho, "r");
  if (!f)
    return 0;

  char linha[MAX_PALAVRA * 4];
  while (fgets(linha, sizeof(linha), f)) {
    trim(linha);
    if (linha[0] == '\0' || linha[0] == '#')
      continue;
    if (linha[0] == '/' && linha[1] == '/')
      continue;
    char *tok = strtok(linha, " \t\r\n");
    while (tok) {
      adicionar_palavra(lista, tok);
      tok = strtok(NULL, " \t\r\n");
    }
  }
  fclose(f);
  return 1;
}

static void ler_palavras_terminal(ListaPalavras *lista) {
  char linha[MAX_PALAVRA * 4];
  printf("\nDigite as palavras (uma por linha ou separadas por espaco).\n");
  printf("Digite 'fim' para encerrar a entrada e iniciar os testes.\n");
  printf("Use '_' para a palavra vazia e '=palavra' para testar 'fim'/'_' literalmente.\n");

  for (;;) {
    printf("> ");
    if (!fgets(linha, sizeof(linha), stdin))
      break;
    trim(linha);
    if (eh_palavra_parada(linha))
      break;
    if (linha[0] == '\0')
      continue;
    char *tok = strtok(linha, " \t\r\n");
    while (tok) {
      adicionar_palavra(lista, tok);
      tok = strtok(NULL, " \t\r\n");
    }
  }
}

static void fecho_lambda(const AutomatoJson *a, const int *entrada, int *saida) {
  int fila[JSON_MAX_ESTADOS];
  int ini = 0, fim = 0;

  for (int i = 0; i < a->n; i++) {
    saida[i] = entrada[i];
    if (saida[i])
      fila[fim++] = i;
  }

  while (ini < fim) {
    int q = fila[ini++];
    for (int t = 0; t < a->qtd_transicoes; t++) {
      if (!a->transicoes[t].is_lambda || a->transicoes[t].origem != q)
        continue;
      int d = a->transicoes[t].destino;
      if (d >= 0 && d < a->n && !saida[d]) {
        saida[d] = 1;
        fila[fim++] = d;
      }
    }
  }
}

static int simularPalavraAutomatoJson(const AutomatoJson *a, const char *palavra) {
  if (!a || a->n <= 0 || a->n > JSON_MAX_ESTADOS)
    return -1;

  int atual[JSON_MAX_ESTADOS] = {0};
  int proximo[JSON_MAX_ESTADOS] = {0};

  for (int i = 0; i < a->qtd_iniciais; i++) {
    int q = a->iniciais[i];
    if (q >= 0 && q < a->n)
      atual[q] = 1;
  }
  fecho_lambda(a, atual, atual);

  for (const char *p = palavra; *p; p++) {
    char c = *p;
    if (c == '\r' || c == '\n')
      continue;

    int simbolo = -1;
    for (int k = 0; k < a->m; k++) {
      if (a->alfabeto[k] == c) {
        simbolo = k;
        break;
      }
    }
    if (simbolo < 0)
      return -1;

    memset(proximo, 0, sizeof(int) * (size_t)a->n);
    for (int q = 0; q < a->n; q++) {
      if (!atual[q])
        continue;
      for (int t = 0; t < a->qtd_transicoes; t++) {
        if (a->transicoes[t].is_lambda ||
            a->transicoes[t].origem != q ||
            a->transicoes[t].simbolo != c)
          continue;
        int d = a->transicoes[t].destino;
        if (d >= 0 && d < a->n)
          proximo[d] = 1;
      }
    }
    fecho_lambda(a, proximo, atual);
  }

  for (int i = 0; i < a->qtd_finais; i++) {
    int q = a->finais[i];
    if (q >= 0 && q < a->n && atual[q])
      return 1;
  }
  return 0;
}

static void testarPalavras(const ArquivoJsonAutomatos *arq,
                           const ListaPalavras *lista) {
  if (lista->qtd == 0) {
    printf("\nNenhuma palavra para testar.\n");
    pausarTela();
    return;
  }

  const AutomatoJson *automatos[2] = {&arq->a1, &arq->a2};
  int total = arq->qtd_automatos > 2 ? 2 : arq->qtd_automatos;
  int aceitas[2] = {0};

  printf("\n=== Resultados dos testes ===\n");
  for (int i = 0; i < lista->qtd; i++) {
    const char *w = lista->itens[i];
    printf("\nPalavra %d: \"%s\"\n", i + 1, w[0] ? w : "<vazia>");
    for (int k = 0; k < total; k++) {
      int r = simularPalavraAutomatoJson(automatos[k], w);
      if (r == 1) {
        printf("  %-12s -> ACEITA\n", automatos[k]->nome);
        aceitas[k]++;
      } else if (r == 0) {
        printf("  %-12s -> REJEITADA\n", automatos[k]->nome);
      } else {
        printf("  %-12s -> SIMBOLO FORA DO ALFABETO\n", automatos[k]->nome);
      }
    }
  }

  printf("\n=== Resumo ===\n");
  for (int k = 0; k < total; k++)
    printf("%s: %d/%d palavra(s) aceita(s).\n", automatos[k]->nome,
           aceitas[k], lista->qtd);
  pausarTela();
}

void menuSimularAutomato(void) {
  const char *caminhoJson = "input_automatos.json";
  const char *caminhoTxt = "input_palavras.txt";

  limparTela();
  imprimirCabecalho("Simulador de Palavras");

  ArquivoJsonAutomatos arq;
  if (!lerArquivoJson(caminhoJson, &arq)) {
    printf("Erro ao ler o arquivo JSON '%s'.\n", caminhoJson);
    pausarTela();
    return;
  }

  printf("\nAutomatos carregados de '%s':\n", caminhoJson);
  printf("1 - %s (%d estados, %d simbolos)\n", arq.a1.nome, arq.a1.n,
         arq.a1.m);
  if (arq.qtd_automatos >= 2)
    printf("2 - %s (%d estados, %d simbolos)\n", arq.a2.nome, arq.a2.n,
           arq.a2.m);

  printf("\nFonte das palavras a testar:\n");
  printf("1 - Arquivo %s\n", caminhoTxt);
  printf("2 - Digitar no terminal\n");
  printf("3 - Arquivo + terminal\n");
  printf("0 - Voltar\n");
  printf("Opcao: ");

  int modo = -1;
  if (scanf("%d", &modo) != 1)
    modo = -1;
  {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
  }

  if (modo == 0)
    return;
  if (modo < 1 || modo > 3) {
    printf("Opcao invalida.\n");
    pausarTela();
    return;
  }

  static ListaPalavras lista;
  memset(&lista, 0, sizeof(lista));

  if (modo == 1 || modo == 3) {
    if (ler_palavras_arquivo(caminhoTxt, &lista))
      printf("\n%d palavra(s) carregada(s) de '%s'.\n", lista.qtd, caminhoTxt);
    else
      printf("\nNao foi possivel abrir '%s'.\n", caminhoTxt);
  }

  if (modo == 2 || modo == 3)
    ler_palavras_terminal(&lista);

  testarPalavras(&arq, &lista);
}
