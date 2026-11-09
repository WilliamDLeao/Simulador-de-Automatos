#ifndef JSON_READER_H
#define JSON_READER_H

#include "afd.h"
#include "conversor_AFN_para_AFD.h"
#include "afn_multiplos_estados.h"

#define JSON_MAX_ESTADOS 100
#define JSON_MAX_SIMBOLOS 26
#define JSON_MAX_NOME_ESTADO 32

/* Estrutura intermediária que armazena um autômato extraído de JSON */
typedef struct {
  char nome[64];
  int n;
  int m;
  char nomes_estados[JSON_MAX_ESTADOS][JSON_MAX_NOME_ESTADO];
  char alfabeto[JSON_MAX_SIMBOLOS];
  int qtd_iniciais;
  int iniciais[JSON_MAX_ESTADOS];
  int qtd_finais;
  int finais[JSON_MAX_ESTADOS];

  int qtd_transicoes;
  struct {
    int origem;
    int destino;
    char simbolo;
    int is_lambda;
  } transicoes[JSON_MAX_ESTADOS * JSON_MAX_SIMBOLOS * 4];
} AutomatoJson;

/* Armazena os autômatos lidos de um arquivo JSON (pode ter 1 ou 2) */
typedef struct {
  int qtd_automatos; /* 1 ou 2 */
  AutomatoJson a1;   /* autômato1 ou autômato único */
  AutomatoJson a2;   /* autômato2 (se presente) */
} ArquivoJsonAutomatos;

/* Lê o arquivo JSON e preenche a estrutura com 1 ou 2 autômatos. Retorna 1 se ok, 0 se erro. */
int lerArquivoJson(const char *caminho, ArquivoJsonAutomatos *res);

/* Conversores de AutomatoJson para as estruturas do projeto */
int automatoJsonParaAFD(const AutomatoJson *aj, AFD *afd);
int automatoJsonParaAFN(const AutomatoJson *aj, AFN *afn);
int automatoJsonParaAFNMultiplo(const AutomatoJson *aj, AFNMultiplo *afnm);

/* Leitura de 1 autômato de arquivo JSON. Se houver 2 no arquivo e indice_automato == 0,
   pergunta ao usuário qual usar (1 ou 2). Se indice_automato for 1 ou 2, usa diretamente. */
int lerAFDJson(const char *caminho, AFD *a, int indice_automato);
int lerAFNJson(const char *caminho, AFN *a, int indice_automato);
int lerAFNMultiploJson(const char *caminho, AFNMultiplo *a, int indice_automato);

/* Leitura de 2 autômatos de um único arquivo JSON (ex: exemplo_leitura.json com autômato1 e autômato2) */
int lerDoisAFDsJson(const char *caminho, AFD *a1, AFD *a2);
int lerDoisAFNsJson(const char *caminho, AFN *a1, AFN *a2);

#endif
