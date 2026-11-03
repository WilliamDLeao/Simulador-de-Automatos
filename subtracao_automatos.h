#ifndef SUBTRACAO_AUTOMATOS_H
#define SUBTRACAO_AUTOMATOS_H

#include "afd.h"

/* Realiza a subtracao de dois AFDs: L(a1) - L(a2), via Produto Cartesiano.
   O estado par (p, q) e de aceitacao se p in F1 E q NAO in F2.
   Retorna 1 em caso de sucesso, 0 se o limite de estados for excedido. */
int subtrairAFDs(const AFD *a1, const AFD *a2, AFD *saida);

/* Menu interativo da subtracao de automatos (Opcao 8 do menu principal) */
void menuSubtracaoAutomatos(void);

#endif