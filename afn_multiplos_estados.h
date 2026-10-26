#ifndef AFN_MULTIPLOS_ESTADOS_H
#define AFN_MULTIPLOS_ESTADOS_H

#include "afd.h"
#include <stdint.h>

#define MAX_AFN_ME_ESTADOS 64
#define SIMBOLO_LAMBDA 'l'

/* Estrutura de AFN com suporte a multiplos estados iniciais e finais */
typedef struct {
  int n;                                       /* Quantidade de estados (0 .. n-1) */
  int m;                                       /* Tamanho do alfabeto */
  char alfabeto[MAX_SIMBOLOS];                 /* Simbolos do alfabeto */
  int qtd_iniciais;                            /* Quantidade de estados iniciais */
  int inicial[MAX_ESTADOS];                    /* inicial[i] = 1 se 'i' e inicial */
  uint64_t iniciais_mask;                      /* Mascara de bits dos estados iniciais */
  int qtd_finais;                              /* Quantidade de estados finais */
  int final[MAX_ESTADOS];                      /* final[i] = 1 se 'i' e final */
  uint64_t finais_mask;                        /* Mascara de bits dos estados finais */
  uint64_t trans[MAX_ESTADOS][MAX_SIMBOLOS];   /* trans[origem][simbolo] = destinos */
  uint64_t trans_lambda[MAX_ESTADOS];          /* trans_lambda[origem] = destinos via lambda */
} AFNMultiplo;

/* Estrutura de AFN resultante com transicoes lambda e estado inicial unico */
typedef struct {
  int n;                                       /* Quantidade de estados (0 .. n-1) */
  int m;                                       /* Tamanho do alfabeto */
  char alfabeto[MAX_SIMBOLOS];                 /* Simbolos do alfabeto */
  int inicial;                                 /* Estado inicial unico */
  int qtd_finais;                              /* Quantidade de estados finais */
  int final[MAX_ESTADOS];                      /* final[i] = 1 se 'i' e final */
  uint64_t finais_mask;                        /* Mascara de bits dos estados finais */
  uint64_t trans[MAX_ESTADOS][MAX_SIMBOLOS];   /* trans[origem][simbolo] = destinos */
  uint64_t trans_lambda[MAX_ESTADOS];          /* trans_lambda[origem] = destinos via lambda */
} AFNLambda;

/* Funcoes de leitura */
int lerAFNMultiplo(AFNMultiplo *a);
int lerAFNMultiploArquivo(const char *nome, AFNMultiplo *a);

/* Funcoes de exibicao */
void imprimirAFNMultiplo(const AFNMultiplo *a);
void imprimirAFNLambda(const AFNLambda *a);

/* Salva o AFN-lambda no formato compativel com o leitor de afn_lambda */
int salvarAFNLambdaArquivo(const char *nome, const AFNLambda *a);

/* Converte o AFN com multiplos estados iniciais para AFN-lambda com estado inicial unico.
   Se unificar_finais != 0, tambem unifica os multiplos estados finais em um unico estado final via lambda.
   Retorna 1 em caso de sucesso, 0 se exceder o limite de estados. */
int converterMultiplosParaLambda(const AFNMultiplo *entrada, AFNLambda *saida, int unificar_finais);

/* Menu interativo do modulo (para ser chamado pelo main) */
void menuAFNMultiplosParaLambda(void);

#endif
