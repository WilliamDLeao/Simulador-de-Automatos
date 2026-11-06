#ifndef INTERSECCAO_AUTOMATOS_H
#define INTERSECCAO_AUTOMATOS_H

#include "afd.h"
#include "conversor_AFN_para_AFD.h"

/* Constroi os pares alcancaveis de dois AFDs. Aceita transicoes parciais.
   Retorna 1 no sucesso; 0 para entrada invalida ou limites excedidos.
   Em erro, saida permanece intacta. Permite saida == a1 ou saida == a2.
   Limites: MAX_ESTADOS estados e MAX_SIMBOLOS simbolos no resultado. */
int intersectarAFDs(const AFD *a1, const AFD *a2, AFD *saida);
/* Aceita AFNs sem lambda (AFDs tambem), determiniza e calcula a interseccao.
   Mesma garantia de saida intacta em erro. Entradas: ate 64 estados. */
int intersectarAFNs(const AFN *a1, const AFN *a2, AFD *saida);
void menuInterseccaoAutomatos(void);

#endif
