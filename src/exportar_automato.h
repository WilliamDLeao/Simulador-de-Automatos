#ifndef EXPORTAR_AUTOMATO_H
#define EXPORTAR_AUTOMATO_H

#include "afd.h"
#include "conversor_AFN_para_AFD.h"
#include "afn_multiplos_estados.h"
#include "json_reader.h"

/* Exporta AFD para arquivo no formato .txt */
int exportarAFDParaTxt(const char *caminho, const AFD *a);

/* Exporta AFD para arquivo no formato .json */
int exportarAFDParaJson(const char *caminho, const AFD *a, const char *nome_obj);

/* Exporta AFN para arquivo no formato .txt */
int exportarAFNParaTxt(const char *caminho, const AFN *a);

/* Exporta AFN para arquivo no formato .json */
int exportarAFNParaJson(const char *caminho, const AFN *a, const char *nome_obj);

/* Exporta AFNMultiplo para arquivo no formato .txt */
int exportarAFNMultiploParaTxt(const char *caminho, const AFNMultiplo *a);

/* Exporta AFNMultiplo para arquivo no formato .json */
int exportarAFNMultiploParaJson(const char *caminho, const AFNMultiplo *a, const char *nome_obj);

/* Exporta dois AFDs em um único arquivo .json (autômato1 e autômato2) */
int exportarDoisAFDsParaJson(const char *caminho, const AFD *a1, const AFD *a2);

/* Oferece interativamente ao usuário salvar um AFD nos formatos .txt, .json ou ambos */
void oferecerExportacaoAFD(const AFD *a, const char *sugestao_nome);

/* Oferece interativamente ao usuário salvar um AFN nos formatos .txt, .json ou ambos */
void oferecerExportacaoAFN(const AFN *a, const char *sugestao_nome);

/* Menu interativo de exportação (Opção do menu principal) */
void menuExportarAutomato(void);

#endif

