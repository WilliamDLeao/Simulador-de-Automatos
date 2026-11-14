#include "exportar_automato.h"
#include "commons.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void limparBufferEntrada(void) {
  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF)
    ;
}

static int terminaComExtensao(const char *str, const char *ext) {
  if (!str || !ext)
    return 0;
  size_t lenStr = strlen(str);
  size_t lenExt = strlen(ext);
  if (lenStr < lenExt)
    return 0;
  const char *p = str + (lenStr - lenExt);
  for (size_t i = 0; i < lenExt; i++) {
    char c1 = (char)tolower((unsigned char)p[i]);
    char c2 = (char)tolower((unsigned char)ext[i]);
    if (c1 != c2)
      return 0;
  }
  return 1;
}

static void garantirExtensao(char *destino, size_t tam, const char *origem,
                             const char *ext) {
  if (!destino || tam == 0 || !origem || !ext)
    return;
  strncpy(destino, origem, tam - 1);
  destino[tam - 1] = '\0';
  if (!terminaComExtensao(destino, ext)) {
    if (strlen(destino) + strlen(ext) < tam) {
      strcat(destino, ext);
    }
  }
}

static void obterNomeBase(char *destino, size_t tam, const char *origem) {
  if (!destino || tam == 0 || !origem)
    return;
  strncpy(destino, origem, tam - 1);
  destino[tam - 1] = '\0';
  char *ponto = strrchr(destino, '.');
  if (ponto && (terminaComExtensao(ponto, ".txt") ||
                terminaComExtensao(ponto, ".json"))) {
    *ponto = '\0';
  }
}

int exportarAFDParaTxt(const char *caminho, const AFD *a) {
  if (!caminho || !a)
    return 0;
  FILE *f = fopen(caminho, "w");
  if (!f) {
    printf("Erro ao criar o arquivo '%s'.\n", caminho);
    return 0;
  }

  /* 1) n m */
  fprintf(f, "%d %d\n", a->n, a->m);

  /* 2) simbolos do alfabeto */
  for (int s = 0; s < a->m; s++) {
    fprintf(f, "%c%c", a->alfabeto[s], (s == a->m - 1) ? '\n' : ' ');
  }

  /* 3) estado inicial */
  fprintf(f, "%d\n", a->inicial);

  /* 4) qtd_finais f1 f2 ... */
  int nf = 0;
  for (int i = 0; i < a->n; i++) {
    if (a->final[i])
      nf++;
  }
  fprintf(f, "%d", nf);
  for (int i = 0; i < a->n; i++) {
    if (a->final[i])
      fprintf(f, " %d", i);
  }
  fprintf(f, "\n");

  /* 5) transicoes: origem simbolo destino */
  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      if (a->delta[i][s] >= 0) {
        fprintf(f, "%d %c %d\n", i, a->alfabeto[s], a->delta[i][s]);
      }
    }
  }

  fclose(f);
  return 1;
}

int exportarAFDParaJson(const char *caminho, const AFD *a,
                        const char *nome_obj) {
  if (!caminho || !a)
    return 0;
  FILE *f = fopen(caminho, "w");
  if (!f) {
    printf("Erro ao criar o arquivo '%s'.\n", caminho);
    return 0;
  }
  if (!nome_obj || !*nome_obj)
    nome_obj = "autômato1";

  fprintf(f, "{\n");
  fprintf(f, "  \"%s\": {\n", nome_obj);

  /* alfabeto */
  fprintf(f, "    \"alfabeto\": [");
  for (int s = 0; s < a->m; s++) {
    fprintf(f, "\"%c\"%s", a->alfabeto[s], (s == a->m - 1) ? "" : ", ");
  }
  fprintf(f, "],\n");

  /* estados */
  fprintf(f, "    \"estados\": [");
  for (int i = 0; i < a->n; i++) {
    fprintf(f, "\"Q%d\"%s", i, (i == a->n - 1) ? "" : ", ");
  }
  fprintf(f, "],\n");

  /* estados finais */
  fprintf(f, "    \"estados finais\": [");
  int primeiro = 1;
  for (int i = 0; i < a->n; i++) {
    if (a->final[i]) {
      if (!primeiro)
        fprintf(f, ", ");
      fprintf(f, "\"Q%d\"", i);
      primeiro = 0;
    }
  }
  fprintf(f, "],\n");

  /* estados iniciais */
  fprintf(f, "    \"estados iniciais\": [\"Q%d\"],\n", a->inicial);

  /* transições */
  fprintf(f, "    \"transições\": [\n");
  primeiro = 1;
  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      if (a->delta[i][s] >= 0) {
        if (!primeiro)
          fprintf(f, ",\n");
        fprintf(f, "      [\"Q%d\", \"Q%d\", \"%c\"]", i, a->delta[i][s],
                a->alfabeto[s]);
        primeiro = 0;
      }
    }
  }
  fprintf(f, "\n    ]\n");
  fprintf(f, "  }\n");
  fprintf(f, "}\n");

  fclose(f);
  return 1;
}

int exportarAFNParaTxt(const char *caminho, const AFN *a) {
  if (!caminho || !a)
    return 0;
  FILE *f = fopen(caminho, "w");
  if (!f) {
    printf("Erro ao criar o arquivo '%s'.\n", caminho);
    return 0;
  }

  /* 1) n m */
  fprintf(f, "%d %d\n", a->n, a->m);

  /* 2) simbolos do alfabeto */
  for (int s = 0; s < a->m; s++) {
    fprintf(f, "%c%c", a->alfabeto[s], (s == a->m - 1) ? '\n' : ' ');
  }

  /* 3) estado inicial */
  fprintf(f, "%d\n", a->inicial);

  /* 4) qtd_finais f1 f2 ... */
  int nf = 0;
  for (int i = 0; i < a->n; i++) {
    if (a->final[i])
      nf++;
  }
  fprintf(f, "%d", nf);
  for (int i = 0; i < a->n; i++) {
    if (a->final[i])
      fprintf(f, " %d", i);
  }
  fprintf(f, "\n");

  /* 5) transicoes */
  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      for (int d = 0; d < a->n; d++) {
        if (a->trans[i][s] & (1ULL << d)) {
          fprintf(f, "%d %c %d\n", i, a->alfabeto[s], d);
        }
      }
    }
  }

  fclose(f);
  return 1;
}

int exportarAFNParaJson(const char *caminho, const AFN *a,
                        const char *nome_obj) {
  if (!caminho || !a)
    return 0;
  FILE *f = fopen(caminho, "w");
  if (!f) {
    printf("Erro ao criar o arquivo '%s'.\n", caminho);
    return 0;
  }
  if (!nome_obj || !*nome_obj)
    nome_obj = "autômato1";

  fprintf(f, "{\n");
  fprintf(f, "  \"%s\": {\n", nome_obj);

  /* alfabeto */
  fprintf(f, "    \"alfabeto\": [");
  for (int s = 0; s < a->m; s++) {
    fprintf(f, "\"%c\"%s", a->alfabeto[s], (s == a->m - 1) ? "" : ", ");
  }
  fprintf(f, "],\n");

  /* estados */
  fprintf(f, "    \"estados\": [");
  for (int i = 0; i < a->n; i++) {
    fprintf(f, "\"Q%d\"%s", i, (i == a->n - 1) ? "" : ", ");
  }
  fprintf(f, "],\n");

  /* estados finais */
  fprintf(f, "    \"estados finais\": [");
  int primeiro = 1;
  for (int i = 0; i < a->n; i++) {
    if (a->final[i]) {
      if (!primeiro)
        fprintf(f, ", ");
      fprintf(f, "\"Q%d\"", i);
      primeiro = 0;
    }
  }
  fprintf(f, "],\n");

  /* estados iniciais */
  fprintf(f, "    \"estados iniciais\": [\"Q%d\"],\n", a->inicial);

  /* transições */
  fprintf(f, "    \"transições\": [\n");
  primeiro = 1;
  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      for (int d = 0; d < a->n; d++) {
        if (a->trans[i][s] & (1ULL << d)) {
          if (!primeiro)
            fprintf(f, ",\n");
          fprintf(f, "      [\"Q%d\", \"Q%d\", \"%c\"]", i, d,
                  a->alfabeto[s]);
          primeiro = 0;
        }
      }
    }
  }
  fprintf(f, "\n    ]\n");
  fprintf(f, "  }\n");
  fprintf(f, "}\n");

  fclose(f);
  return 1;
}

int exportarAFNMultiploParaTxt(const char *caminho, const AFNMultiplo *a) {
  if (!caminho || !a)
    return 0;
  FILE *f = fopen(caminho, "w");
  if (!f) {
    printf("Erro ao criar o arquivo '%s'.\n", caminho);
    return 0;
  }

  fprintf(f, "%d %d\n", a->n, a->m);

  for (int s = 0; s < a->m; s++) {
    fprintf(f, "%c%s", a->alfabeto[s], s == a->m - 1 ? "" : " ");
  }
  fprintf(f, "\n");

  /* Iniciais */
  fprintf(f, "%d", a->qtd_iniciais);
  for (int i = 0; i < a->n; i++) {
    if (a->inicial[i])
      fprintf(f, " %d", i);
  }
  fprintf(f, "\n");

  /* Finais */
  fprintf(f, "%d", a->qtd_finais);
  for (int i = 0; i < a->n; i++) {
    if (a->final[i])
      fprintf(f, " %d", i);
  }
  fprintf(f, "\n");

  /* Transicoes */
  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      for (int d = 0; d < a->n; d++) {
        if (a->trans[i][s] & (1ULL << d))
          fprintf(f, "%d %c %d\n", i, a->alfabeto[s], d);
      }
    }
    for (int d = 0; d < a->n; d++) {
      if (a->trans_lambda[i] & (1ULL << d))
        fprintf(f, "%d l %d\n", i, d);
    }
  }

  fclose(f);
  return 1;
}

int exportarAFNMultiploParaJson(const char *caminho, const AFNMultiplo *a,
                                const char *nome_obj) {
  if (!caminho || !a)
    return 0;
  FILE *f = fopen(caminho, "w");
  if (!f) {
    printf("Erro ao criar o arquivo '%s'.\n", caminho);
    return 0;
  }
  if (!nome_obj || !*nome_obj)
    nome_obj = "autômato1";

  fprintf(f, "{\n");
  fprintf(f, "  \"%s\": {\n", nome_obj);

  /* alfabeto */
  fprintf(f, "    \"alfabeto\": [");
  for (int s = 0; s < a->m; s++) {
    fprintf(f, "\"%c\"%s", a->alfabeto[s], (s == a->m - 1) ? "" : ", ");
  }
  fprintf(f, "],\n");

  /* estados */
  fprintf(f, "    \"estados\": [");
  for (int i = 0; i < a->n; i++) {
    fprintf(f, "\"Q%d\"%s", i, (i == a->n - 1) ? "" : ", ");
  }
  fprintf(f, "],\n");

  /* estados finais */
  fprintf(f, "    \"estados finais\": [");
  int primeiro = 1;
  for (int i = 0; i < a->n; i++) {
    if (a->final[i]) {
      if (!primeiro)
        fprintf(f, ", ");
      fprintf(f, "\"Q%d\"", i);
      primeiro = 0;
    }
  }
  fprintf(f, "],\n");

  /* estados iniciais */
  fprintf(f, "    \"estados iniciais\": [");
  primeiro = 1;
  for (int i = 0; i < a->n; i++) {
    if (a->inicial[i]) {
      if (!primeiro)
        fprintf(f, ", ");
      fprintf(f, "\"Q%d\"", i);
      primeiro = 0;
    }
  }
  fprintf(f, "],\n");

  /* transições */
  fprintf(f, "    \"transições\": [\n");
  primeiro = 1;
  for (int i = 0; i < a->n; i++) {
    for (int s = 0; s < a->m; s++) {
      for (int d = 0; d < a->n; d++) {
        if (a->trans[i][s] & (1ULL << d)) {
          if (!primeiro)
            fprintf(f, ",\n");
          fprintf(f, "      [\"Q%d\", \"Q%d\", \"%c\"]", i, d,
                  a->alfabeto[s]);
          primeiro = 0;
        }
      }
    }
    for (int d = 0; d < a->n; d++) {
      if (a->trans_lambda[i] & (1ULL << d)) {
        if (!primeiro)
          fprintf(f, ",\n");
        fprintf(f, "      [\"Q%d\", \"Q%d\", \"l\"]", i, d);
        primeiro = 0;
      }
    }
  }
  fprintf(f, "\n    ]\n");
  fprintf(f, "  }\n");
  fprintf(f, "}\n");

  fclose(f);
  return 1;
}

int exportarDoisAFDsParaJson(const char *caminho, const AFD *a1,
                             const AFD *a2) {
  if (!caminho || !a1 || !a2)
    return 0;
  FILE *f = fopen(caminho, "w");
  if (!f) {
    printf("Erro ao criar o arquivo '%s'.\n", caminho);
    return 0;
  }
  fprintf(f, "{\n");

  /* Bloco automato 1 */
  fprintf(f, "  \"autômato1\": {\n");
  fprintf(f, "    \"alfabeto\": [");
  for (int s = 0; s < a1->m; s++) {
    fprintf(f, "\"%c\"%s", a1->alfabeto[s], (s == a1->m - 1) ? "" : ", ");
  }
  fprintf(f, "],\n");
  fprintf(f, "    \"estados\": [");
  for (int i = 0; i < a1->n; i++) {
    fprintf(f, "\"Q%d\"%s", i, (i == a1->n - 1) ? "" : ", ");
  }
  fprintf(f, "],\n");
  fprintf(f, "    \"estados finais\": [");
  int primeiro = 1;
  for (int i = 0; i < a1->n; i++) {
    if (a1->final[i]) {
      if (!primeiro)
        fprintf(f, ", ");
      fprintf(f, "\"Q%d\"", i);
      primeiro = 0;
    }
  }
  fprintf(f, "],\n");
  fprintf(f, "    \"estados iniciais\": [\"Q%d\"],\n", a1->inicial);
  fprintf(f, "    \"transições\": [\n");
  primeiro = 1;
  for (int i = 0; i < a1->n; i++) {
    for (int s = 0; s < a1->m; s++) {
      if (a1->delta[i][s] >= 0) {
        if (!primeiro)
          fprintf(f, ",\n");
        fprintf(f, "      [\"Q%d\", \"Q%d\", \"%c\"]", i, a1->delta[i][s],
                a1->alfabeto[s]);
        primeiro = 0;
      }
    }
  }
  fprintf(f, "\n    ]\n  },\n");

  /* Bloco automato 2 */
  fprintf(f, "  \"autômato2\": {\n");
  fprintf(f, "    \"alfabeto\": [");
  for (int s = 0; s < a2->m; s++) {
    fprintf(f, "\"%c\"%s", a2->alfabeto[s], (s == a2->m - 1) ? "" : ", ");
  }
  fprintf(f, "],\n");
  fprintf(f, "    \"estados\": [");
  for (int i = 0; i < a2->n; i++) {
    fprintf(f, "\"Q%d\"%s", i, (i == a2->n - 1) ? "" : ", ");
  }
  fprintf(f, "],\n");
  fprintf(f, "    \"estados finais\": [");
  primeiro = 1;
  for (int i = 0; i < a2->n; i++) {
    if (a2->final[i]) {
      if (!primeiro)
        fprintf(f, ", ");
      fprintf(f, "\"Q%d\"", i);
      primeiro = 0;
    }
  }
  fprintf(f, "],\n");
  fprintf(f, "    \"estados iniciais\": [\"Q%d\"],\n", a2->inicial);
  fprintf(f, "    \"transições\": [\n");
  primeiro = 1;
  for (int i = 0; i < a2->n; i++) {
    for (int s = 0; s < a2->m; s++) {
      if (a2->delta[i][s] >= 0) {
        if (!primeiro)
          fprintf(f, ",\n");
        fprintf(f, "      [\"Q%d\", \"Q%d\", \"%c\"]", i, a2->delta[i][s],
                a2->alfabeto[s]);
        primeiro = 0;
      }
    }
  }
  fprintf(f, "\n    ]\n  }\n");
  fprintf(f, "}\n");

  fclose(f);
  return 1;
}

void oferecerExportacaoAFD(const AFD *a, const char *sugestao_nome) {
  if (!a)
    return;
  printf("\nDeseja exportar este AFD?\n");
  printf("1 - Exportar como .txt\n");
  printf("2 - Exportar como .json\n");
  printf("3 - Exportar como .txt e .json\n");
  printf("0 - Nao exportar\n");
  printf("Opcao: ");
  int op = 0;
  if (scanf("%d", &op) != 1 || op <= 0 || op > 3) {
    limparBufferEntrada();
    return;
  }

  char nome[256];
  printf("Nome base do arquivo (ex: %s): ",
         sugestao_nome ? sugestao_nome : "resultado");
  if (scanf("%255s", nome) != 1)
    return;

  char base[256];
  obterNomeBase(base, sizeof(base), nome);

  if (op == 1 || op == 3) {
    char caminho_txt[300];
    garantirExtensao(caminho_txt, sizeof(caminho_txt), base, ".txt");
    if (exportarAFDParaTxt(caminho_txt, a)) {
      printf("AFD salvo com sucesso em '%s'.\n", caminho_txt);
    }
  }
  if (op == 2 || op == 3) {
    char caminho_json[300];
    garantirExtensao(caminho_json, sizeof(caminho_json), base, ".json");
    if (exportarAFDParaJson(caminho_json, a, "autômato1")) {
      printf("AFD salvo com sucesso em '%s'.\n", caminho_json);
    }
  }
}

void oferecerExportacaoAFN(const AFN *a, const char *sugestao_nome) {
  if (!a)
    return;
  printf("\nDeseja exportar este AFN?\n");
  printf("1 - Exportar como .txt\n");
  printf("2 - Exportar como .json\n");
  printf("3 - Exportar como .txt e .json\n");
  printf("0 - Nao exportar\n");
  printf("Opcao: ");
  int op = 0;
  if (scanf("%d", &op) != 1 || op <= 0 || op > 3) {
    limparBufferEntrada();
    return;
  }

  char nome[256];
  printf("Nome base do arquivo (ex: %s): ",
         sugestao_nome ? sugestao_nome : "resultado");
  if (scanf("%255s", nome) != 1)
    return;

  char base[256];
  obterNomeBase(base, sizeof(base), nome);

  if (op == 1 || op == 3) {
    char caminho_txt[300];
    garantirExtensao(caminho_txt, sizeof(caminho_txt), base, ".txt");
    if (exportarAFNParaTxt(caminho_txt, a)) {
      printf("AFN salvo com sucesso em '%s'.\n", caminho_txt);
    }
  }
  if (op == 2 || op == 3) {
    char caminho_json[300];
    garantirExtensao(caminho_json, sizeof(caminho_json), base, ".json");
    if (exportarAFNParaJson(caminho_json, a, "autômato1")) {
      printf("AFN salvo com sucesso em '%s'.\n", caminho_json);
    }
  }
}

void menuExportarAutomato(void) {
  limparTela();
  imprimirCabecalho("EXPORTAR AUTOMATO (.TXT / .JSON)");

  printf("Escolha a origem do automato a ser exportado:\n");
  printf("1 - Ler de arquivo .txt\n");
  printf("2 - Ler de arquivo .json\n");
  printf("3 - Digitar AFD manualmente pelo teclado\n");
  printf("4 - Digitar AFN manualmente pelo teclado\n");
  printf("0 - Voltar ao menu principal\n");
  printf("Opcao: ");

  int opOrigem = -1;
  if (scanf("%d", &opOrigem) != 1 || opOrigem == 0) {
    limparBufferEntrada();
    return;
  }

  AFD afd;
  AFN afn;
  int eh_afd = 0, eh_afn = 0;
  char caminho_origem[256];

  if (opOrigem == 1) {
    printf("Nome do arquivo .txt (ex: ex_leitura.txt): ");
    if (scanf("%255s", caminho_origem) != 1)
      return;

    FILE *teste = fopen(caminho_origem, "r");
    if (!teste) {
      printf("\nNao foi possivel abrir o arquivo '%s'.\n", caminho_origem);
      limparBufferEntrada();
      pausarTela();
      return;
    }
    fclose(teste);

    /* Tenta ler como AFD primeiro */
    if (lerAFDArquivo(caminho_origem, &afd)) {
      eh_afd = 1;
      printf("\nAutômato carregado como AFD com sucesso!\n");
      imprimirAFD(&afd);
    } else if (lerAFNArquivo(caminho_origem, &afn)) {
      eh_afn = 1;
      printf("\nAutômato carregado como AFN com sucesso!\n");
      imprimirAFN(&afn);
    } else {
      printf("\nErro ao ler o arquivo '%s' como AFD ou AFN.\n",
             caminho_origem);
      limparBufferEntrada();
      pausarTela();
      return;
    }
  } else if (opOrigem == 2) {
    printf("Nome do arquivo .json (ex: exemplo_leitura.json): ");
    if (scanf("%255s", caminho_origem) != 1)
      return;

    ArquivoJsonAutomatos aj;
    if (!lerArquivoJson(caminho_origem, &aj)) {
      printf("\nErro ao abrir ou ler arquivo JSON '%s'.\n", caminho_origem);
      limparBufferEntrada();
      pausarTela();
      return;
    }

    int idx = 1;
    if (aj.qtd_automatos > 1) {
      printf("\nO arquivo JSON contem 2 automatos:\n");
      printf("1 - %s\n", aj.a1.nome[0] ? aj.a1.nome : "automato1");
      printf("2 - %s\n", aj.a2.nome[0] ? aj.a2.nome : "automato2");
      printf("3 - Exportar ambos em um novo arquivo .json\n");
      printf("Escolha a opcao (1, 2 ou 3) [1]: ");
      if (scanf("%d", &idx) != 1 || idx < 1 || idx > 3)
        idx = 1;

      if (idx == 3) {
        AFD a1, a2;
        if (automatoJsonParaAFD(&aj.a1, &a1) &&
            automatoJsonParaAFD(&aj.a2, &a2)) {
          char nome_saida[256];
          printf("Nome do arquivo .json de destino (ex: dois_automatos.json): ");
          if (scanf("%255s", nome_saida) == 1) {
            char caminho_saida[300];
            garantirExtensao(caminho_saida, sizeof(caminho_saida), nome_saida,
                             ".json");
            if (exportarDoisAFDsParaJson(caminho_saida, &a1, &a2)) {
              printf("\nAmbos os automatos foram exportados para '%s' com "
                     "sucesso!\n",
                     caminho_saida);
            }
          }
        } else {
          printf("\nNao foi possivel converter ambos os automatos em AFD para "
                 "exportacao conjunta.\n");
        }
        limparBufferEntrada();
        pausarTela();
        return;
      }
    }

    const AutomatoJson *escolhido = (idx == 2) ? &aj.a2 : &aj.a1;

    /* Verifica se possui transicoes nao-deterministicas ou lambdas */
    int tem_nd = (escolhido->qtd_iniciais > 1);
    for (int t1 = 0; t1 < escolhido->qtd_transicoes && !tem_nd; t1++) {
      if (escolhido->transicoes[t1].is_lambda) {
        tem_nd = 1;
        break;
      }
      for (int t2 = t1 + 1; t2 < escolhido->qtd_transicoes; t2++) {
        if (escolhido->transicoes[t1].origem ==
                escolhido->transicoes[t2].origem &&
            escolhido->transicoes[t1].simbolo ==
                escolhido->transicoes[t2].simbolo) {
          tem_nd = 1;
          break;
        }
      }
    }

    if (!tem_nd && automatoJsonParaAFD(escolhido, &afd)) {
      eh_afd = 1;
      printf("\nAutomato '%s' carregado como AFD com sucesso!\n",
             escolhido->nome);
      imprimirAFD(&afd);
    } else if (automatoJsonParaAFN(escolhido, &afn)) {
      eh_afn = 1;
      printf("\nAutomato '%s' carregado como AFN com sucesso!\n",
             escolhido->nome);
      imprimirAFN(&afn);
    } else {
      printf("\nNao foi possivel converter o automato do JSON.\n");
      limparBufferEntrada();
      pausarTela();
      return;
    }
  } else if (opOrigem == 3) {
    printf("\nDigite os dados do AFD:\n");
    if (!lerAFD(&afd)) {
      printf("\nErro ao ler AFD do teclado.\n");
      limparBufferEntrada();
      pausarTela();
      return;
    }
    eh_afd = 1;
    printf("\nAFD inserido com sucesso:\n");
    imprimirAFD(&afd);
  } else if (opOrigem == 4) {
    printf("\nDigite os dados do AFN:\n");
    if (!lerAFN(&afn)) {
      printf("\nErro ao ler AFN do teclado.\n");
      limparBufferEntrada();
      pausarTela();
      return;
    }
    eh_afn = 1;
    printf("\nAFN inserido com sucesso:\n");
    imprimirAFN(&afn);
  } else {
    limparBufferEntrada();
    return;
  }

  if (eh_afd) {
    oferecerExportacaoAFD(&afd, "automato_exportado");
  } else if (eh_afn) {
    oferecerExportacaoAFN(&afn, "automato_exportado");
  }

  limparBufferEntrada();
  pausarTela();
}

