#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "conversor_AFN_para_AFD.h"

void inicializar_afn(AFN *afn) {
    if (!afn) return;
    afn->num_estados_nfa = 0;
    afn->num_simbolos = 0;
    afn->estado_inicial = 0;
    afn->num_finais_nfa = 0;
    afn->finais_nfa_mask = 0;
    for (int i = 0; i < MAX_SIMBOLOS; i++) {
        afn->simbolos[i] = '\0';
    }
    for (int i = 0; i < MAX_NFA_ESTADOS; i++) {
        for (int j = 0; j < MAX_SIMBOLOS; j++) {
            afn->trans_nfa[i][j] = 0; // 0 significa nenhuma transição
        }
    }
}

void inicializar_afd(AFD *dfa) {
    if (!dfa) return;
    dfa->total_dfa_estados = 0;
    for (int i = 0; i < MAX_DFA_ESTADOS; i++) {
        dfa->dfa_estados[i] = 0;
        dfa->dfa_aceitacao[i] = false;
        for (int j = 0; j < MAX_SIMBOLOS; j++) {
            dfa->dfa_trans[i][j] = -1; // -1 significa sem transição (ou estado morto)
        }
    }
}

int obter_indice_simbolo(const AFN *afn, char c) {
    for (int i = 0; i < afn->num_simbolos; i++) {
        if (afn->simbolos[i] == c) {
            return i;
        }
    }
    // Fallback: se o símbolo for numérico '0', '1', etc. e não estiver explicitado
    if (isdigit(c)) {
        int idx = c - '0';
        if (idx >= 0 && idx < afn->num_simbolos) {
            return idx;
        }
    }
    return -1;
}

// Lê a próxima linha útil (ignorando linhas vazias e comentários com '#' ou '//')
static bool ler_linha_util(FILE *f, char *buffer, int max_tam) {
    while (fgets(buffer, max_tam, f) != NULL) {
        // Remove quebra de linha
        buffer[strcspn(buffer, "\r\n")] = '\0';

        // Pula espaços iniciais
        char *ptr = buffer;
        while (*ptr && isspace((unsigned char)*ptr)) {
            ptr++;
        }

        // Se vazia ou comentário, continua
        if (*ptr == '\0' || *ptr == '#' || (*ptr == '/' && *(ptr + 1) == '/')) {
            continue;
        }

        // Copia a linha sem espaços iniciais de volta para o buffer
        if (ptr != buffer) {
            memmove(buffer, ptr, strlen(ptr) + 1);
        }
        return true;
    }
    return false;
}

int carregar_afn_arquivo(const char *caminho_arquivo, AFN *afn) {
    FILE *f = fopen(caminho_arquivo, "r");
    if (!f) {
        printf("\n[ERRO] Nao foi possivel abrir o arquivo: %s\n", caminho_arquivo);
        return 0;
    }

    inicializar_afn(afn);
    char buffer[256];

    // 1. Quantidade de estados do AFN
    if (!ler_linha_util(f, buffer, sizeof(buffer))) {
        printf("\n[ERRO] Arquivo vazio ou sem definicao de quantidade de estados!\n");
        fclose(f);
        return 0;
    }
    if (sscanf(buffer, "%d", &afn->num_estados_nfa) != 1 || afn->num_estados_nfa <= 0) {
        printf("\n[ERRO] Quantidade de estados invalida: '%s'\n", buffer);
        fclose(f);
        return 0;
    }
    if (afn->num_estados_nfa > MAX_NFA_ESTADOS) {
        printf("\n[AVISO] Quantidade de estados (%d) excede o maximo (%d). Limitando a %d.\n",
               afn->num_estados_nfa, MAX_NFA_ESTADOS, MAX_NFA_ESTADOS);
        afn->num_estados_nfa = MAX_NFA_ESTADOS;
    }

    // 2. Alfabeto / Símbolos
    if (!ler_linha_util(f, buffer, sizeof(buffer))) {
        printf("\n[ERRO] Fim inesperado do arquivo ao ler o alfabeto!\n");
        fclose(f);
        return 0;
    }

    // Coleta todos os tokens da linha do alfabeto
    char *tokens_alfabeto[MAX_SIMBOLOS + 5];
    int total_tokens_alf = 0;
    char *token = strtok(buffer, " \t,");
    while (token != NULL && total_tokens_alf < (MAX_SIMBOLOS + 5)) {
        tokens_alfabeto[total_tokens_alf++] = token;
        token = strtok(NULL, " \t,");
    }

    if (total_tokens_alf == 0) {
        afn->simbolos[0] = 'a';
        afn->simbolos[1] = 'b';
        afn->num_simbolos = 2;
    } else if (total_tokens_alf == 1 && isdigit(tokens_alfabeto[0][0])) {
        // Apenas um número K (ex: "2"), cria símbolos 'a', 'b', ...
        int k = atoi(tokens_alfabeto[0]);
        if (k > MAX_SIMBOLOS) k = MAX_SIMBOLOS;
        for (int s = 0; s < k; s++) {
            afn->simbolos[s] = 'a' + s;
        }
        afn->num_simbolos = k;
    } else {
        // Verifica se o primeiro token é uma contagem explícita (ex: "2 a b" ou "2 0 1")
        int inicio = 0;
        if (isdigit(tokens_alfabeto[0][0]) && total_tokens_alf > 1) {
            int k = atoi(tokens_alfabeto[0]);
            if (k == total_tokens_alf - 1) {
                inicio = 1; // primeiro token era de fato a quantidade
            }
        }

        int count_simb = 0;
        for (int i = inicio; i < total_tokens_alf && count_simb < MAX_SIMBOLOS; i++) {
            afn->simbolos[count_simb++] = tokens_alfabeto[i][0];
        }
        afn->num_simbolos = count_simb;
    }

    // 3. Estados finais
    if (!ler_linha_util(f, buffer, sizeof(buffer))) {
        printf("\n[ERRO] Fim inesperado do arquivo ao ler os estados finais!\n");
        fclose(f);
        return 0;
    }

    // Lê os números presentes na linha dos estados finais
    int finais_lidos[MAX_NFA_ESTADOS];
    int total_finais_linha = 0;
    token = strtok(buffer, " \t,");
    while (token != NULL && total_finais_linha < MAX_NFA_ESTADOS) {
        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
            int est = atoi(token);
            finais_lidos[total_finais_linha++] = est;
        }
        token = strtok(NULL, " \t,");
    }

    if (total_finais_linha > 0) {
        // Se o primeiro número for a contagem e bater com o total restante:
        // Ex: "2 1 2" (quantidade=2, estados=1,2)
        if (total_finais_linha > 1 && finais_lidos[0] == total_finais_linha - 1) {
            for (int i = 1; i < total_finais_linha; i++) {
                int est = finais_lidos[i];
                if (est >= 0 && est < afn->num_estados_nfa) {
                    afn->finais_nfa_mask |= (1 << est);
                    afn->num_finais_nfa++;
                }
            }
        } else {
            // Todos os números são estados finais diretamente (ex: "2" ou "1 2")
            for (int i = 0; i < total_finais_linha; i++) {
                int est = finais_lidos[i];
                if (est >= 0 && est < afn->num_estados_nfa) {
                    afn->finais_nfa_mask |= (1 << est);
                    afn->num_finais_nfa++;
                }
            }
        }
    }

    // 4. Transições do AFN
    // Formato: origem simbolo destino1 destino2 ...
    // Exemplo: 0 a 0 1
    //          0 b 0
    //          1 b 2
    int count_transicoes = 0;
    while (ler_linha_util(f, buffer, sizeof(buffer))) {
        token = strtok(buffer, " \t,");
        if (!token) continue;
        int origem = atoi(token);

        token = strtok(NULL, " \t,");
        if (!token) continue;
        char simb = token[0];

        int idx_simb = obter_indice_simbolo(afn, simb);
        if (idx_simb == -1) {
            printf("\n[AVISO] Simbolo '%c' na transicao nao pertence ao alfabeto! Ignorando.\n", simb);
            continue;
        }

        // Lê todos os destinos na linha
        while ((token = strtok(NULL, " \t,")) != NULL) {
            if (strcmp(token, "-") == 0) continue; // '-' representa conjunto vazio
            int destino = atoi(token);
            if (destino >= 0 && destino < afn->num_estados_nfa && origem >= 0 && origem < afn->num_estados_nfa) {
                afn->trans_nfa[origem][idx_simb] |= (1 << destino);
                count_transicoes++;
            }
        }
    }

    fclose(f);
    printf("\n[SUCESSO] AFN carregado com sucesso de '%s' (%d transicoes lidas).\n", caminho_arquivo, count_transicoes);
    return 1;
}

void converter_afn_para_afd(AFN *afn, AFD *dfa) {
    inicializar_afd(dfa);
    int fila_inicio = 0, fila_fim = 0;

    // Estado inicial do AFD é o conjunto contendo o estado inicial do AFN (geralmente q0)
    int estado_ini = (afn->estado_inicial >= 0 && afn->estado_inicial < afn->num_estados_nfa) 
                     ? afn->estado_inicial : 0;
    dfa->dfa_estados[0] = (1 << estado_ini);
    dfa->total_dfa_estados = 1;
    fila_fim = 1;

    // Processamento em Largura (BFS) para Construção de Subconjuntos
    while (fila_inicio < fila_fim) {
        int atual_index = fila_inicio++;
        int atual_mask = dfa->dfa_estados[atual_index];

        // Verifica se o estado do AFD é final (contém pelo menos um final do AFN)
        dfa->dfa_aceitacao[atual_index] = ((atual_mask & afn->finais_nfa_mask) != 0);

        // Para cada símbolo do alfabeto
        for (int j = 0; j < afn->num_simbolos; j++) {
            int proximo_mask = 0;

            // Para cada estado do AFN presente no subconjunto atual
            for (int e = 0; e < afn->num_estados_nfa; e++) {
                if (atual_mask & (1 << e)) {
                    proximo_mask |= afn->trans_nfa[e][j];
                }
            }

            // Se o conjunto de destino for vazio
            if (proximo_mask == 0) {
                dfa->dfa_trans[atual_index][j] = -1; // -1 representa sem transição / estado vazio
                continue;
            }

            // Verifica se o novo conjunto já existe na lista de estados do AFD
            int index_existente = -1;
            for (int k = 0; k < dfa->total_dfa_estados; k++) {
                if (dfa->dfa_estados[k] == proximo_mask) {
                    index_existente = k;
                    break;
                }
            }

            // Se não existe, adiciona como um novo estado do AFD
            if (index_existente == -1) {
                if (dfa->total_dfa_estados < MAX_DFA_ESTADOS) {
                    index_existente = dfa->total_dfa_estados;
                    dfa->dfa_estados[index_existente] = proximo_mask;
                    dfa->total_dfa_estados++;
                    fila_fim++;
                } else {
                    printf("\n[AVISO] Limite de estados do AFD (%d) atingido!\n", MAX_DFA_ESTADOS);
                }
            }

            dfa->dfa_trans[atual_index][j] = index_existente;
        }
    }
}

void imprimir_subconjunto(int mask, int max_estados) {
    if (mask == 0) {
        printf("    { }   ");
        return;
    }
    printf("{");
    bool primeiro = true;
    for (int e = 0; e < max_estados; e++) {
        if (mask & (1 << e)) {
            if (!primeiro) printf(",");
            printf("q%d", e);
            primeiro = false;
        }
    }
    printf("}");
}

void imprimir_afn(const AFN *afn) {
    printf("\n============================================================\n");
    printf("              AUTOMATO COM FINITOS NAO-DETERMINISTICO (AFN)\n");
    printf("============================================================\n");
    printf("  Estados: %d (q0 a q%d)\n", afn->num_estados_nfa, afn->num_estados_nfa - 1);
    printf("  Alfabeto: { ");
    for (int i = 0; i < afn->num_simbolos; i++) {
        printf("'%c'%s", afn->simbolos[i], (i < afn->num_simbolos - 1) ? ", " : " ");
    }
    printf("}\n");
    printf("  Estado Inicial: -> q%d\n", afn->estado_inicial);
    printf("  Estados Finais: { ");
    bool primeiro = true;
    for (int e = 0; e < afn->num_estados_nfa; e++) {
        if (afn->finais_nfa_mask & (1 << e)) {
            if (!primeiro) printf(", ");
            printf("q%d", e);
            primeiro = false;
        }
    }
    printf(" }\n\n");

    printf("  Tabela de Transicoes do AFN:\n");
    printf("  %-10s |", "Estado");
    for (int j = 0; j < afn->num_simbolos; j++) {
        printf("    '%c'     |", afn->simbolos[j]);
    }
    printf("\n");
    printf("  -----------+");
    for (int j = 0; j < afn->num_simbolos; j++) {
        printf("------------+");
    }
    printf("\n");

    for (int i = 0; i < afn->num_estados_nfa; i++) {
        char prefixo[8] = "   ";
        if (i == afn->estado_inicial && (afn->finais_nfa_mask & (1 << i))) {
            strcpy(prefixo, "->*");
        } else if (i == afn->estado_inicial) {
            strcpy(prefixo, "-> ");
        } else if (afn->finais_nfa_mask & (1 << i)) {
            strcpy(prefixo, " * ");
        }

        printf("  %s q%-4d |", prefixo, i);
        for (int j = 0; j < afn->num_simbolos; j++) {
            int mask = afn->trans_nfa[i][j];
            if (mask == 0) {
                printf("     -      |");
            } else {
                char temp[32];
                int pos = 0;
                temp[pos++] = '{';
                bool p = true;
                for (int e = 0; e < afn->num_estados_nfa; e++) {
                    if (mask & (1 << e)) {
                        if (!p) temp[pos++] = ',';
                        pos += sprintf(temp + pos, "q%d", e);
                        p = false;
                    }
                }
                temp[pos++] = '}';
                temp[pos] = '\0';
                printf(" %-10s |", temp);
            }
        }
        printf("\n");
    }
    printf("  Legenda: '->' Inicial, '*' Final, '-' Sem transicao\n");
}

void imprimir_afd(const AFN *afn, const AFD *dfa) {
    printf("\n============================================================\n");
    printf("              AUTOMATO COM FINITOS DETERMINISTICO (AFD)\n");
    printf("============================================================\n");
    printf("  Total de estados no AFD gerado: %d\n\n", dfa->total_dfa_estados);

    printf("  Mapeamento de Estados (Subconjuntos do AFN):\n");
    for (int i = 0; i < dfa->total_dfa_estados; i++) {
        printf("    S%-2d = ", i);
        imprimir_subconjunto(dfa->dfa_estados[i], afn->num_estados_nfa);
        if (dfa->dfa_aceitacao[i]) {
            printf("  [FINAL]");
        }
        printf("\n");
    }
    printf("\n");

    printf("  Tabela de Transicoes do AFD:\n");
    printf("  %-10s | %-16s |", "Estado", "Subconjunto");
    for (int j = 0; j < afn->num_simbolos; j++) {
        printf("   '%c'   |", afn->simbolos[j]);
    }
    printf(" Aceitacao?\n");

    printf("  -----------+------------------+");
    for (int j = 0; j < afn->num_simbolos; j++) {
        printf("---------+");
    }
    printf("------------\n");

    for (int i = 0; i < dfa->total_dfa_estados; i++) {
        char prefixo[8] = "   ";
        if (i == 0 && dfa->dfa_aceitacao[i]) {
            strcpy(prefixo, "->*");
        } else if (i == 0) {
            strcpy(prefixo, "-> ");
        } else if (dfa->dfa_aceitacao[i]) {
            strcpy(prefixo, " * ");
        }

        // Formata string do subconjunto
        char subconj[32];
        int pos = 0;
        subconj[pos++] = '{';
        bool p = true;
        for (int e = 0; e < afn->num_estados_nfa; e++) {
            if (dfa->dfa_estados[i] & (1 << e)) {
                if (!p) subconj[pos++] = ',';
                pos += sprintf(subconj + pos, "q%d", e);
                p = false;
            }
        }
        subconj[pos++] = '}';
        subconj[pos] = '\0';

        printf("  %s S%-4d | %-16s |", prefixo, i, subconj);

        for (int j = 0; j < afn->num_simbolos; j++) {
            int dest = dfa->dfa_trans[i][j];
            if (dest == -1) {
                printf("    -    |");
            } else {
                printf("   S%-3d |", dest);
            }
        }

        printf("    %s\n", dfa->dfa_aceitacao[i] ? "SIM" : "NAO");
    }
    printf("  Legenda: '->' Inicial, '*' Final, '-' Sem transicao (vazio)\n");
}

bool simular_cadeia_afd(const AFN *afn, const AFD *dfa, const char *cadeia) {
    if (dfa->total_dfa_estados <= 0) {
        printf("\n[ERRO] Nenhum AFD carregado para simulacao!\n");
        return false;
    }

    printf("\n--- Simulando Cadeia \"%s\" no AFD ---\n", cadeia);
    int estado_atual = 0; // S0 é sempre o estado inicial

    printf("  Inicio: S0 ");
    imprimir_subconjunto(dfa->dfa_estados[0], afn->num_estados_nfa);
    printf("\n");

    int tam = strlen(cadeia);
    if (tam == 0) {
        printf("  (Cadeia vazia)\n");
    }

    for (int i = 0; i < tam; i++) {
        char c = cadeia[i];
        int idx = obter_indice_simbolo(afn, c);
        if (idx == -1) {
            printf("  [REJEITADA] Simbolo '%c' nao pertence ao alfabeto!\n", c);
            return false;
        }

        int proximo = dfa->dfa_trans[estado_atual][idx];
        if (proximo == -1) {
            printf("  Passo %d: S%d lendo '%c' -> Sem transicao (caiu em erro/vazio).\n", i + 1, estado_atual, c);
            printf("  [REJEITADA] Nao ha transicao definida a partir de S%d com '%c'.\n", estado_atual, c);
            return false;
        }

        printf("  Passo %d: S%d --('%c')--> S%d ", i + 1, estado_atual, c, proximo);
        imprimir_subconjunto(dfa->dfa_estados[proximo], afn->num_estados_nfa);
        printf("\n");

        estado_atual = proximo;
    }

    bool aceita = dfa->dfa_aceitacao[estado_atual];
    if (aceita) {
        printf("  Resultado: O estado final atingido S%d e de ACEITACAO!\n", estado_atual);
        printf("  >>> [CADEIA ACEITA] <<<\n");
    } else {
        printf("  Resultado: O estado final atingido S%d NAO e de aceitacao.\n", estado_atual);
        printf("  >>> [CADEIA REJEITADA] <<<\n");
    }
    return aceita;
}

int salvar_afd_arquivo(const char *caminho_arquivo, const AFN *afn, const AFD *dfa) {
    FILE *f = fopen(caminho_arquivo, "w");
    if (!f) {
        printf("\n[ERRO] Nao foi possivel criar o arquivo: %s\n", caminho_arquivo);
        return 0;
    }

    fprintf(f, "# ============================================================\n");
    fprintf(f, "# AUTOMATO FINITO DETERMINISTICO (AFD) CONVERTIDO DO AFN\n");
    fprintf(f, "# ============================================================\n\n");

    fprintf(f, "# Quantidade total de estados do AFD:\n%d\n\n", dfa->total_dfa_estados);

    fprintf(f, "# Simbolos do alfabeto:\n");
    for (int j = 0; j < afn->num_simbolos; j++) {
        fprintf(f, "%c ", afn->simbolos[j]);
    }
    fprintf(f, "\n\n");

    fprintf(f, "# Estados de aceitacao (Finais) do AFD:\n");
    for (int i = 0; i < dfa->total_dfa_estados; i++) {
        if (dfa->dfa_aceitacao[i]) {
            fprintf(f, "%d ", i);
        }
    }
    fprintf(f, "\n\n");

    fprintf(f, "# Mapeamento dos Estados (Subconjuntos do AFN):\n");
    for (int i = 0; i < dfa->total_dfa_estados; i++) {
        fprintf(f, "# S%d = {", i);
        bool primeiro = true;
        for (int e = 0; e < afn->num_estados_nfa; e++) {
            if (dfa->dfa_estados[i] & (1 << e)) {
                if (!primeiro) fprintf(f, ", ");
                fprintf(f, "q%d", e);
                primeiro = false;
            }
        }
        fprintf(f, "}%s\n", dfa->dfa_aceitacao[i] ? " [FINAL]" : "");
    }
    fprintf(f, "\n");

    fprintf(f, "# Transicoes do AFD (origem simbolo destino):\n");
    for (int i = 0; i < dfa->total_dfa_estados; i++) {
        for (int j = 0; j < afn->num_simbolos; j++) {
            int dest = dfa->dfa_trans[i][j];
            if (dest != -1) {
                fprintf(f, "%d %c %d\n", i, afn->simbolos[j], dest);
            }
        }
    }

    fclose(f);
    printf("\n[SUCESSO] AFD salvo com sucesso em '%s'!\n", caminho_arquivo);
    return 1;
}