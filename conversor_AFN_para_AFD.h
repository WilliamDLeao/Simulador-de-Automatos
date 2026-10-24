#ifndef CONVERSOR_AFN_AFD_H
#define CONVERSOR_AFN_AFD_H

#include "afd.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_AFN_ESTADOS 64

/* Estrutura do AFN (Automato com Finitos Nao-Deterministico)
   n: quantidade de estados (0..n-1)
   m: tamanho do alfabeto
   alfabeto: simbolos
   inicial: estado inicial
   final[i]: 1 se i e final, 0 caso contrario
   finais_mask: mascara de bits dos estados finais
   trans[i][s]: mascara de bits dos estados de destino da transicao delta(i, s)
*/
typedef struct {
  int n;                                       /* quantidade de estados   */
  int m;                                       /* tamanho do alfabeto     */
  char alfabeto[MAX_SIMBOLOS];                 /* simbolos                */
  int inicial;                                 /* estado inicial          */
  int final[MAX_ESTADOS];                      /* final[i] = 1 se i final */
  uint64_t finais_mask;                        /* mascara de bits dos finais */
  uint64_t trans[MAX_ESTADOS][MAX_SIMBOLOS];   /* transicoes por bitmask  */
} AFN;

/* Funcoes de Inicializacao e Auxiliares */
void inicializar_afn(AFN *afn);
void inicializar_afd(AFD *dfa);
int obter_indice_simbolo(const AFN *afn, char c);
int obter_indice_simbolo_afd(const AFD *dfa, char c);
int adicionarTransicaoAFN(AFN *a, int o, char sim, int d);

/* Leitura generica compativel com o padrao afd.c */
int lerAFN(AFN *a);                                  /* le do teclado com perguntas */
int lerAFNArquivo(const char *nome, AFN *a);         /* le de arquivo .txt no formato padrao */
int carregar_afn_arquivo(const char *caminho, AFN *a); /* alias retrocompativel */

/* Conversao AFN -> AFD via Algoritmo de Construcao de Subconjuntos */
int converter_afn_para_afd_subconjuntos(const AFN *afn, AFD *dfa, uint64_t *subconjuntos);
int converter_afn_para_afd(AFN *afn, AFD *dfa);      /* versao de 2 argumentos */

/* Impressao */
void imprimir_afn(const AFN *afn);
void imprimir_subconjunto(uint64_t mask, int max_estados);
void imprimir_afd(const AFN *afn, const AFD *dfa);
void imprimir_afd_com_subconjuntos(const AFN *afn, const AFD *dfa, const uint64_t *subconjuntos);

/* Simulacao de Cadeias */
bool simular_cadeia_afd(const AFN *afn, const AFD *dfa, const char *cadeia);
bool simular_cadeia_afd_detalhada(const AFN *afn, const AFD *dfa, const uint64_t *subconjuntos, const char *cadeia);

/* Exportacao de AFD (formato padrao do projeto, compativel com lerAFDArquivo) */
int salvar_afd_arquivo(const char *caminho_arquivo, const AFN *afn, const AFD *dfa);
int salvarAFDArquivo(const char *caminho_arquivo, const AFD *dfa);

/* Menu interativo */
void menuConversorAFNparaAFD(void);

#endif