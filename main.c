#include "commons.h"
#include "afn_multiplos_estados.h"
#include "conversor_AFN_para_AFD.h"
#include "afn_lambda.h"
#include "minimizar_afd.h"
#include <stdio.h>
#include <stdlib.h>


int main() {
  int op = -1;

  do {
    limparTela();
    printf("\n----Menu----");
    printf("\nEscolha seu algoritmo:");
    printf("\n1-Converter AFN com multiplos estados para AFN-lambda");
    printf("\n2-AFN lambda para AFN");
    printf("\n3-Converter AFN para AFD");
    printf("\n4-Minimizar AFD");
    printf("\n0-Sair");
    printf("\nDigite o número: ");

    int leitura = scanf("%d", &op);
    if (leitura == EOF) return 0;
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
      while ((ch = getchar()) != '\n' && ch != EOF) {}
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
    case 0:
      break;
    default:
      break;
    }
  } while (op != 0);

  return 0;
}