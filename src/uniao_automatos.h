#ifndef UNIAO_AUTOMATOS_H
#define UNIAO_AUTOMATOS_H

#include "afd.h"

/* Realiza a união de dois AFDs via Produto Cartesiano (construção em paralelo).
   O estado par (p, q) é de aceitação se p in F1 OU q in F2.
   Retorna 1 em caso de sucesso, 0 se o limite de estados for excedido. */
int unirAFDs(const AFD *a1, const AFD *a2, AFD *saida);

/* Menu interativo da união de autômatos (Case 6) */
void menuUniaoAutomatos(void);

#endif
