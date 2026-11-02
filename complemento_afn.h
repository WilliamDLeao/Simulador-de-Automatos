#ifndef COMPLEMENTO_AFN_H
#define COMPLEMENTO_AFN_H

#include "afd.h"
#include "conversor_AFN_para_AFD.h"

/* Determina um AFD equivalente e completo a partir de um AFN.
   Garante que todas as transicoes estejam definidas, incluindo estado
   morto/poço caso haja transicoes indefinidas.
   Retorna 1 em caso de sucesso, 0 se o limite de estados for excedido. */
int determinizarAFNCompleto(const AFN *afn, AFD *dfa_completo);

/* Inverte os estados finais de um AFD completo (F' = Q \ F).
   Estados que eram finais tornam-se nao-finais e vice-versa. */
void inverterEstadosFinaisAFD(const AFD *entrada, AFD *saida);

/* Converte um AFD para a estrutura de AFN equivalente.
   Retorna 1 em caso de sucesso, 0 se dfa->n > MAX_AFN_ESTADOS. */
int converterAFDparaAFN(const AFD *dfa, AFN *afn);

/* Realiza a operacao de complemento formal do AFN:
   1. Converte o AFN para AFD via construcao de subconjuntos;
   2. Completa a funcao de transicao com estado morto (se necessario);
   3. Inverte os estados de aceitacao (F' = Q \ F).
   Se afd_comp != NULL, armazena o AFD complementar gerado.
   Se afn_comp != NULL, armazena a versao AFN equivalente.
   Retorna 1 em caso de sucesso, 0 em caso de erro/limite excedido. */
int complementarAFN(const AFN *afn, AFD *afd_comp, AFN *afn_comp);

/* Salva um AFD em arquivo .txt compativel com o simulador */
int salvarAFDEmArquivo(const char *nome, const AFD *a);

/* Salva um AFN em arquivo .txt compativel com o simulador */
int salvarAFNEmArquivo(const char *nome, const AFN *a);

/* Simula uma palavra em um AFD.
   Retorna 1 se aceita, 0 se rejeitada, -1 se contem simbolo fora do alfabeto. */
int simularPalavraAFD(const AFD *a, const char *palavra);

/* Menu interativo da operacao de complemento de AFN (Opcao 9 do menu principal) */
void menuComplementoAFN(void);

#endif
