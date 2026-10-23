#ifndef MINIMIZAR_AFD_H
#define MINIMIZAR_AFD_H

#include "afd.h"

/* Minimiza o AFD 'entrada' (algoritmo de Moore) e grava o resultado em 'saida'.
   Retorna 1 se ok, 0 se nao coube (limite MAX_ESTADOS). */
int minimizarAFD(const AFD *entrada, AFD *saida);

/* Menu: le o AFD, minimiza e imprime. Chamar do main. */
void menuMinimizarAFD(void);

#endif