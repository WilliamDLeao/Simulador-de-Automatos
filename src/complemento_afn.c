#include "complemento_afn.h"
#include "commons.h"
#include "exportar_automato.h"
#include "minimizar_afd.h"
#include "json_reader.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int determinizarAFNCompleto(const AFN *afn, AFD *dfa_completo) {
  if (!converterAFNparaAFD(afn, dfa_completo)) {
    return 0;
  }

  /* Verifica se ha transicoes indefinidas (delta == -1) */
  int faltando = 0;
  for (int i = 0; i < dfa_completo->n; i++) {
    for (int s = 0; s < dfa_completo->m; s++) {
      if (dfa_completo->delta[i][s] < 0) {
        faltando = 1;
        break;
      }
    }
    if (faltando)
      break;
  }

  /* Se houver transicoes faltando, completa com estado morto/poço */
  if (faltando) {
    if (dfa_completo->n >= MAX_ESTADOS) {
      return 0;
    }
    int morto = dfa_completo->n;
    dfa_completo->final[morto] = 0;

    /* Transicoes do estado morto levam a ele mesmo */
    for (int s = 0; s < dfa_completo->m; s++) {
      dfa_completo->delta[morto][s] = morto;
    }

    /* Redireciona transicoes indefinidas para o estado morto */
    for (int i = 0; i < dfa_completo->n; i++) {
      for (int s = 0; s < dfa_completo->m; s++) {
        if (dfa_completo->delta[i][s] < 0) {
          dfa_completo->delta[i][s] = morto;
        }
      }
    }
    dfa_completo->n++;
  }

  return 1;
}

void inverterEstadosFinaisAFD(const AFD *entrada, AFD *saida) {
  *saida = *entrada;
  for (int i = 0; i < saida->n; i++) {
    saida->final[i] = (entrada->final[i] == 0) ? 1 : 0;
  }
}

int converterAFDparaAFN(const AFD *dfa, AFN *afn) {
  if (dfa->n > MAX_AFN_ESTADOS) {
    return 0;
  }

  afn->n = dfa->n;
  afn->m = dfa->m;
  afn->inicial = dfa->inicial;
  afn->finais_mask = 0;

  for (int s = 0; s < dfa->m; s++) {
    afn->alfabeto[s] = dfa->alfabeto[s];
  }

  for (int i = 0; i < dfa->n; i++) {
    afn->final[i] = dfa->final[i];
    if (dfa->final[i]) {
      afn->finais_mask |= (1ULL << i);
    }
    for (int s = 0; s < dfa->m; s++) {
      if (dfa->delta[i][s] >= 0) {
        afn->trans[i][s] = (1ULL << dfa->delta[i][s]);
      } else {
        afn->trans[i][s] = 0;
      }
    }
  }
  return 1;
}

int complementarAFN(const AFN *afn, AFD *afd_comp, AFN *afn_comp) {
  AFD dfa_total, dfa_result;

  if (!determinizarAFNCompleto(afn, &dfa_total)) {
    return 0;
  }

  inverterEstadosFinaisAFD(&dfa_total, &dfa_result);

  if (afd_comp != NULL) {
    *afd_comp = dfa_result;
  }

  if (afn_comp != NULL) {
    if (!converterAFDparaAFN(&dfa_result, afn_comp)) {
      /* Se exceder 64 estados suportados pela bitmask do AFN */
      if (afd_comp == NULL)
        return 0;
    }
  }

  return 1;
}

int salvarAFDEmArquivo(const char *nome, const AFD *a) {
  FILE *f = fopen(nome, "w");
  if (!f) {
    printf("Erro ao criar o arquivo '%s'.\n", nome);
    return 0;
  }

  /* 1) n m */
  fprintf(f, "%d %d\n", a->n, a->m);

  /* 2) simbolos */
  for (int s = 0; s < a->m; s++) {
    fprintf(f, "%c%c", a->alfabeto[s], (s == a->m - 1) ? '\n' : ' ');
  }

  /* 3) inicial */
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

int salvarAFNEmArquivo(const char *nome, const AFN *a) {
  FILE *f = fopen(nome, "w");
  if (!f) {
    printf("Erro ao criar o arquivo '%s'.\n", nome);
    return 0;
  }

  /* 1) n m */
  fprintf(f, "%d %d\n", a->n, a->m);

  /* 2) simbolos */
  for (int s = 0; s < a->m; s++) {
    fprintf(f, "%c%c", a->alfabeto[s], (s == a->m - 1) ? '\n' : ' ');
  }

  /* 3) inicial */
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

int simularPalavraAFD(const AFD *a, const char *palavra) {
  int atual = a->inicial;
  int tam = (int)strlen(palavra);

  for (int i = 0; i < tam; i++) {
    char c = palavra[i];
    if (c == '\r' || c == '\n')
      continue;
    int s = -1;
    for (int k = 0; k < a->m; k++) {
      if (a->alfabeto[k] == c) {
        s = k;
        break;
      }
    }
    if (s < 0) {
      return -1; /* simbolo fora do alfabeto */
    }
    atual = a->delta[atual][s];
    if (atual < 0) {
      return 0; /* sem transicao definida (rejeita) */
    }
  }
  return (a->final[atual] == 1) ? 1 : 0;
}

void menuComplementoAFN(void) {
  AFN afn_orig;
  AFD dfa_completo, dfa_comp, dfa_min;
  AFN afn_comp;

  limparTela();
  imprimirCabecalho("COMPLEMENTO DE AFN");

  printf("Conceito Teorico:\n");
  printf("Em um AFN, um caminho pode rejeitar uma palavra enquanto outro a aceita.\n");
  printf("Logo, simplesmente inverter os estados finais de um AFN NAO gera o complemento.\n");
  printf("Procedimento formal correto:\n");
  printf("  1. Converter AFN -> AFD equivalente (Construcao de Subconjuntos);\n");
  printf("  2. Completar a funcao delta (adicionar estado morto para transicoes vazias);\n");
  printf("  3. Inverter os estados de aceitacao (F' = Q \\ F);\n");
  printf("  4. Opcionalmente minimizar o AFD resultante via Moore.\n\n");

  int opcao, ok;
  char nome[256];

  printf("Como deseja informar o AFN?\n");
  printf("1 - Digitar pelo teclado\n");
  printf("2 - Ler de arquivo .txt\n");
  printf("3 - Ler de arquivo .json\n");
  printf("Opcao: ");
  if (scanf("%d", &opcao) != 1)
    opcao = 1;

  if (opcao == 2) {
    printf("Nome do arquivo (ex: exemplo_afn_ab.txt): ");
    scanf("%255s", nome);
    if (strstr(nome, ".json") != NULL) {
      ok = lerAFNJson(nome, &afn_orig, 0);
    } else {
      ok = lerAFNArquivo(nome, &afn_orig);
    }
  } else if (opcao == 3) {
    printf("Nome do arquivo .json (ex: exemplo_leitura.json): ");
    scanf("%255s", nome);
    ok = lerAFNJson(nome, &afn_orig, 0);
  } else {
    printf("\n");
    ok = lerAFN(&afn_orig);
  }

  if (!ok) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  if (!determinizarAFNCompleto(&afn_orig, &dfa_completo)) {
    printf("\nErro: Limite de estados excedido durante a determinizacao completa.\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  if (!complementarAFN(&afn_orig, &dfa_comp, &afn_comp)) {
    printf("\nErro ao calcular o complemento do AFN.\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
      ;
    pausarTela();
    return;
  }

  printf("\n============================================\n");
  printf("--- 1) AFN ORIGINAL ---\n");
  printf("============================================\n");
  imprimirAFN(&afn_orig);

  printf("\n============================================\n");
  printf("--- 2) AFD DETERMINIZADO E COMPLETO ---\n");
  printf("============================================\n");
  imprimirAFD(&dfa_completo);

  printf("\n============================================\n");
  printf("--- 3) AFD COMPLEMENTAR (F' = Q \\ F) ---\n");
  printf("============================================\n");
  imprimirAFD(&dfa_comp);

  if (afn_comp.n > 0) {
    printf("\n============================================\n");
    printf("--- 4) AFN COMPLEMENTAR EQUIVALENTE ---\n");
    printf("============================================\n");
    imprimirAFN(&afn_comp);
  }

  int tem_minimo = minimizarAFD(&dfa_comp, &dfa_min);
  if (tem_minimo) {
    printf("\n============================================\n");
    printf("--- 5) AFD COMPLEMENTAR MINIMIZADO (MOORE) ---\n");
    printf("============================================\n");
    imprimirAFD(&dfa_min);
  }

  /* Menu de acoes pos-calculo: Testar palavras ou Salvar */
  int sub_op = 0;
  do {
    printf("\nOpcoes adicionais:\n");
    printf("1 - Testar palavras no AFN Original vs Complementar\n");
    printf("2 - Salvar AFD Complementar em arquivo .txt\n");
    printf("3 - Salvar AFN Complementar em arquivo .txt\n");
    printf("4 - Salvar AFD Complementar em arquivo .json\n");
    printf("5 - Salvar AFN Complementar em arquivo .json\n");
    printf("0 - Voltar ao menu principal\n");
    printf("Escolha: ");
    if (scanf("%d", &sub_op) != 1) {
      sub_op = 0;
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF)
        ;
      break;
    }

    if (sub_op == 1) {
      char palavra[256];
      printf("\nDigite uma palavra para testar (ou '_' para palavra vazia, 'fim' para sair):\n");
      while (1) {
        printf("Palavra: ");
        scanf("%255s", palavra);
        if (strcmp(palavra, "fim") == 0)
          break;
        if (strcmp(palavra, "_") == 0 || strcmp(palavra, "lambda") == 0 || strcmp(palavra, "vazio") == 0)
          palavra[0] = '\0';

        int res_orig = simularPalavraAFD(&dfa_completo, palavra);
        int res_comp = simularPalavraAFD(&dfa_comp, palavra);

        const char *p_mostra = (palavra[0] == '\0') ? "<vazia>" : palavra;
        printf("-> Palavra \"%s\":\n", p_mostra);
        if (res_orig < 0) {
          printf("   AFN Original: Contem simbolo invalido (fora do alfabeto)!\n");
        } else {
          printf("   AFN Original       : %s\n", res_orig ? "ACEITA [L(M)]" : "REJEITADA");
        }
        if (res_comp < 0) {
          printf("   AFN Complementar   : Contem simbolo invalido (fora do alfabeto)!\n");
        } else {
          printf("   AFN Complementar   : %s\n", res_comp ? "ACEITA [Complemento]" : "REJEITADA");
        }
        printf("   Consistencia: %s\n\n",
               (res_orig >= 0 && res_comp >= 0 && (res_orig != res_comp))
                   ? "CORRETO (Original != Complementar)"
                   : "ATENCAO");
      }
    } else if (sub_op == 2) {
      char arquivo_salvar[256];
      printf("Nome do arquivo para salvar o AFD (ex: afd_complementar.txt): ");
      scanf("%255s", arquivo_salvar);
      if (salvarAFDEmArquivo(arquivo_salvar, &dfa_comp)) {
        printf("AFD complementar salvo em '%s' com sucesso!\n", arquivo_salvar);
      }
    } else if (sub_op == 3) {
      char arquivo_salvar[256];
      printf("Nome do arquivo para salvar o AFN (ex: afn_complementar.txt): ");
      scanf("%255s", arquivo_salvar);
      if (salvarAFNEmArquivo(arquivo_salvar, &afn_comp)) {
        printf("AFN complementar salvo em '%s' com sucesso!\n", arquivo_salvar);
      }
    } else if (sub_op == 4) {
      char arquivo_salvar[256];
      printf("Nome do arquivo para salvar o AFD em JSON (ex: afd_complementar.json): ");
      scanf("%255s", arquivo_salvar);
      if (exportarAFDParaJson(arquivo_salvar, &dfa_comp, "autômato1")) {
        printf("AFD complementar salvo em '%s' com sucesso!\n", arquivo_salvar);
      }
    } else if (sub_op == 5) {
      char arquivo_salvar[256];
      printf("Nome do arquivo para salvar o AFN em JSON (ex: afn_complementar.json): ");
      scanf("%255s", arquivo_salvar);
      if (exportarAFNParaJson(arquivo_salvar, &afn_comp, "autômato1")) {
        printf("AFN complementar salvo em '%s' com sucesso!\n", arquivo_salvar);
      }
    }
  } while (sub_op != 0);

  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF)
    ;
  pausarTela();
}
