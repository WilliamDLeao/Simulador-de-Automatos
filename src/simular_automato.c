#include "simular_automato.h"
#include "commons.h"
#include "complemento_afn.h"
#include "json_reader.h"
#include <stdio.h>
#include <string.h>

#define MAX_PALAVRA 512

static void simularComAFD(const AFD *afd) {
  int modo;
  printf("\nComo deseja testar?\n");
  printf("1 - Digitar palavra no terminal\n");
  printf("2 - Ler palavras de arquivo .txt\n");
  printf("Opcao: ");
  if (scanf("%d", &modo) != 1)
    return;
  {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
  }

  if (modo == 1) {
    char palavra[MAX_PALAVRA];
    printf("Digite palavras para testar (\"sair\" para encerrar):\n");
    while (1) {
      printf("> ");
      if (!fgets(palavra, sizeof(palavra), stdin))
        break;
      /* remove \n */
      palavra[strcspn(palavra, "\r\n")] = '\0';
      if (strcmp(palavra, "sair") == 0)
        break;
      /* _ representa palavra vazia */
      const char *w = (strcmp(palavra, "_") == 0) ? "" : palavra;
      int r = simularPalavraAFD(afd, w);
      if (r == 1)
        printf("  -> ACEITA\n");
      else if (r == 0)
        printf("  -> REJEITADA\n");
      else
        printf("  -> SIMBOLO FORA DO ALFABETO\n");
    }
  } else if (modo == 2) {
    char caminho[256];
    printf("Caminho do arquivo .txt: ");
    if (!fgets(caminho, sizeof(caminho), stdin))
      return;
    caminho[strcspn(caminho, "\r\n")] = '\0';

    FILE *f = fopen(caminho, "r");
    if (!f) {
      printf("Erro ao abrir '%s'.\n", caminho);
      pausarTela();
      return;
    }

    char palavra[MAX_PALAVRA];
    int total = 0, aceitas = 0;
    while (fgets(palavra, sizeof(palavra), f)) {
      palavra[strcspn(palavra, "\r\n")] = '\0';
      if (palavra[0] == '\0')
        continue; /* linha vazia */
      const char *w = (strcmp(palavra, "_") == 0) ? "" : palavra;
      int r = simularPalavraAFD(afd, w);
      total++;
      if (r == 1) {
        printf("  %-20s -> ACEITA\n", palavra);
        aceitas++;
      } else if (r == 0) {
        printf("  %-20s -> REJEITADA\n", palavra);
      } else {
        printf("  %-20s -> SIMBOLO FORA DO ALFABETO\n", palavra);
      }
    }
    fclose(f);
    printf("\nResultado: %d/%d palavras aceitas.\n", aceitas, total);
    pausarTela();
  } else {
    printf("Opcao invalida.\n");
    pausarTela();
  }
}

void menuSimularAutomato(void) {
  limparTela();
  imprimirCabecalho("Simulador de Palavras");
  printf("=== Simulador de Palavras ===\n");

  const char *caminho = "input_automatos.json";

  ArquivoJsonAutomatos arq;
  if (!lerArquivoJson(caminho, &arq)) {
    printf("Erro ao ler o arquivo JSON.\n");
    pausarTela();
    return;
  }

  AutomatoJson *escolhido = &arq.a1;
  if (arq.qtd_automatos == 2) {
    printf("\nO arquivo contem 2 automatos:\n");
    printf("1 - %s\n", arq.a1.nome);
    printf("2 - %s\n", arq.a2.nome);
    printf("Qual deseja simular? ");
    int op;
    if (scanf("%d", &op) != 1)
      op = 1;
    {
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF)
        ;
    }
    if (op == 2)
      escolhido = &arq.a2;
  }

  printf("\nAutomato selecionado: %s\n", escolhido->nome);
  printf("Estados: %d | Alfabeto: ", escolhido->n);
  for (int i = 0; i < escolhido->m; i++)
    printf("%c ", escolhido->alfabeto[i]);
  printf("\n");

  AFD afd;
  if (!automatoJsonParaAFD(escolhido, &afd)) {
    printf("Erro ao converter automato para AFD.\n");
    pausarTela();
    return;
  }

  simularComAFD(&afd);
}

