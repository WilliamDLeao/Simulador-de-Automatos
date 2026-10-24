#ifndef CONVERSOR_AFN_AFD_H
#define CONVERSOR_AFN_AFD_H

#include "afd.h"
#include <stdint.h>

#define MAX_AFN_ESTADOS 64

/* Estrutura de AFN (estados: 0 .. n-1. trans[i][s] e mascara de bits dos destinos) */
typedef struct {
  int n;                                     /* quantidade de estados   */
  int m;                                     /* tamanho do alfabeto     */
  char alfabeto[MAX_SIMBOLOS];               /* simbolos                */
  int inicial;                               /* estado inicial          */
  int final[MAX_ESTADOS];                    /* final[i] = 1 se i final */
  uint64_t finais_mask;                      /* mascara de bits dos finais */
  uint64_t trans[MAX_ESTADOS][MAX_SIMBOLOS]; /* transicoes por bitmask  */
} AFN;

int lerAFN(AFN *a);                          /* le do teclado, retorna 1 se ok */
int lerAFNArquivo(const char *nome, AFN *a); /* le de um .txt, retorna 1 se ok */
void imprimirAFN(const AFN *a);

/* Converte o AFN em AFD (construcao de subconjuntos). Retorna 1 se ok, 0 se exceder limites. */
int converterAFNparaAFD(const AFN *afn, AFD *dfa);

/* Menu: le o AFN, converte e imprime. Chamar do main. */
void menuConversorAFNparaAFD(void);

#endif