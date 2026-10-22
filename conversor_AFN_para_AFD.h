#ifndef CONVERSOR_AFN_AFD_H
#define CONVERSOR_AFN_AFD_H

#include <stdbool.h>

#define MAX_NFA_ESTADOS 10
#define MAX_DFA_ESTADOS 1024
#define MAX_SIMBOLOS 10

typedef struct {
    int num_estados_nfa;
    int num_simbolos;
    char simbolos[MAX_SIMBOLOS];
    int estado_inicial;
    int num_finais_nfa;
    int trans_nfa[MAX_NFA_ESTADOS][MAX_SIMBOLOS];
    int finais_nfa_mask;
} AFN;

typedef struct {
    int total_dfa_estados;
    int dfa_estados[MAX_DFA_ESTADOS];
    int dfa_trans[MAX_DFA_ESTADOS][MAX_SIMBOLOS];
    bool dfa_aceitacao[MAX_DFA_ESTADOS];
} AFD;

void inicializar_afn(AFN *afn);
void inicializar_afd(AFD *dfa);
int obter_indice_simbolo(const AFN *afn, char c);

int carregar_afn_arquivo(const char *caminho_arquivo, AFN *afn);
void converter_afn_para_afd(AFN *afn, AFD *dfa);

void imprimir_afn(const AFN *afn);
void imprimir_afd(const AFN *afn, const AFD *dfa);
void imprimir_subconjunto(int mask, int max_estados);

bool simular_cadeia_afd(const AFN *afn, const AFD *dfa, const char *cadeia);
int salvar_afd_arquivo(const char *caminho_arquivo, const AFN *afn, const AFD *dfa);

#endif