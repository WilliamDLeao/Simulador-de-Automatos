#include "afn_lambda.h"
#include "afn_multiplos_estados.h"
#include "commons.h"
#include "complemento_afn.h"
#include "conversor_AFN_para_AFD.h"
#include "exportar_automato.h"
#include "interseccao_automatos.h"
#include "minimizar_afd.h"
#include "simular_automato.h"
#include "subtracao_automatos.h"
#include "uniao_automatos.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  int op = -1;

  do {
    limparTela();
    printf("\n----Menu----");
    printf("\nEscolha seu algoritmo:");
    printf("\n1-Converter AFN com multiplos estados para AFN-lambda");
    printf("\n2-AFN lambda para AFN");
    printf("\n3-Converter AFN para AFD");
    printf("\n4-Minimizar AFD");
    printf("\n5-Simular palavras com automato JSON");
    printf("\n6-Uniao de automatos");
    printf("\n7-Interseccao de dois AFs (AFN/AFD)");
    printf("\n8- Subtracao de automatos");
    printf("\n9-Complemento de AFN");
    printf("\n10-Exportar automato (.txt / .json)");
    printf("\n0-Sair");
    printf("\nDigite o número: ");

    int leitura = scanf("%d", &op);
    if (leitura == EOF)
      return 0;
    if (leitura != 1) {
      op = -1; /* entrada invalida (letra) */
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF)
        ;
    }

    switch (op) {
    case 1:
      menuAFNMultiplosParaLambda();
      break;
    case 2: {
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF) {
      }
      transformar_afn_lambda_para_afn();
      break;
    }
    case 3:
      limparTela();
      menuConversorAFNparaAFD();
      break;
    case 4:
      menuMinimizarAFD();
      break;
    case 5: {
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF) {
      }
      menuSimularAutomato();
      break;
    }
    case 10:
      menuExportarAutomato();
      break;
    case 6:
      menuUniaoAutomatos();
      break;
    case 7:
      menuInterseccaoAutomatos();
      break;
    case 8:
      menuSubtracaoAutomatos();
      break;
    case 9:
      menuComplementoAFN();
      break;
    case 0:
      break;
    default:
      break;
    }
  } while (op != 0);

  return 0;
}