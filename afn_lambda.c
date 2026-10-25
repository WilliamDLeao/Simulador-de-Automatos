/* Modulo de conversao de AFN-lambda para AFN com alfabeto informado pelo usuario. C11. */
#include "afn_lambda.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>

#define LIMITE_SIMBOLOS 26

typedef struct {
    int n, m, inicial;
    char alfabeto[LIMITE_SIMBOLOS];
    unsigned char *finais;
    /* Matrizes 0..m-1: simbolos do alfabeto; matriz m: lambda. */
    unsigned char *delta;
} Automato;

#define D(A,S,I,J) ((A)->delta[((size_t)(S)*(A)->n+(I))*(A)->n+(J)])
#define C(I,J) fecho[(size_t)(I)*n+(J)]

static void liberar(Automato *a) {
    free(a->finais);
    free(a->delta);
    memset(a, 0, sizeof *a);
}

static int criar(Automato *a, int n, int m) {
    memset(a, 0, sizeof *a);
    if (n < 1 || n > 100 || m < 1 || m > LIMITE_SIMBOLOS) return 0;
    if ((size_t)n > SIZE_MAX / (size_t)n / (size_t)(m + 1)) {
        puts("Quantidade de estados grande demais.");
        return 0;
    }
    a->n = n;
    a->m = m;
    a->finais = calloc((size_t)n, 1);
    a->delta = calloc((size_t)(m + 1) * n * n, 1);
    if (!a->finais || !a->delta) {
        puts("Memoria insuficiente.");
        liberar(a);
        return 0;
    }
    return 1;
}

/* Descarta o restante de uma linha longa e pede a entrada novamente. */
static int linha(char *buf, int tamanho) {
    for (;;) {
        int ch;
        if (!fgets(buf, tamanho, stdin)) return 0;
        if (strchr(buf, '\n') || feof(stdin)) return 1;
        while ((ch = getchar()) != '\n' && ch != EOF) { }
        puts("Linha muito longa. Digite novamente:");
    }
}

static int inteiro(const char *mensagem, int minimo, int maximo, int *valor) {
    char buf[256], *fim;
    long x;
    for (;;) {
        printf("%s", mensagem);
        fflush(stdout);
        if (!linha(buf, sizeof buf)) return 0;
        errno = 0;
        x = strtol(buf, &fim, 10);
        if (fim != buf && errno != ERANGE) {
            while (isspace((unsigned char)*fim)) fim++;
            if (*fim == '\0' && x >= minimo && x <= maximo) {
                *valor = (int)x;
                return 1;
            }
        }
        printf("Digite um inteiro entre %d e %d.\n", minimo, maximo);
    }
}

/* 'l' fica reservado para lambda; simbolos sao caracteres ASCII visiveis. */
static int indice_simbolo(const Automato *a, char simbolo) {
    if (simbolo == 'l') return a->m;
    for (int i = 0; i < a->m; i++)
        if (a->alfabeto[i] == simbolo) return i;
    return -1;
}

static int adicionar_simbolo(Automato *a, int indice, const char *token) {
    unsigned char c = (unsigned char)token[0];
    if (strlen(token) != 1 || c < 33 || c > 126 || c == 'l') return 0;
    for (int i = 0; i < indice; i++)
        if (a->alfabeto[i] == (char)c) return 0;
    a->alfabeto[indice] = (char)c;
    return 1;
}

static int cadastrar(Automato *a) {
    int n, m, quantidade, estado, i, origem, destino, simbolo;
    char buf[256];
    if (!inteiro("Quantidade de estados: ", 1, 100, &n)) return 0;
    if (!inteiro("Quantidade de simbolos do alfabeto (1 a 26): ", 1,
                 LIMITE_SIMBOLOS, &m) || !criar(a, n, m)) return 0;
    puts("Informe um simbolo por linha (ASCII visivel, sem repetir; l reservado para lambda).");
    for (i = 0; i < m; i++) {
        for (;;) {
            printf("Simbolo %d: ", i + 1);
            fflush(stdout);
            if (!linha(buf, sizeof buf)) return 0;
            buf[strcspn(buf, "\r\n")] = '\0';
            if (adicionar_simbolo(a, i, buf)) break;
            puts("Simbolo invalido ou repetido. Use um caractere visivel diferente de l.");
        }
    }
    printf("Estados numerados de 0 a %d (q0 a q%d).\n", n-1, n-1);
    if (!inteiro("Estado inicial (numero): ", 0, n-1, &a->inicial) ||
        !inteiro("Quantidade de estados finais: ", 0, n, &quantidade)) return 0;
    for (i = 0; i < quantidade; i++) {
        if (!inteiro("Estado final (numero): ", 0, n-1, &estado)) return 0;
        if (a->finais[estado]) {
            puts("Esse estado ja foi informado. Escolha outro.");
            i--;
        } else a->finais[estado] = 1;
    }
    if (!inteiro("Quantidade de transicoes: ", 0, INT_MAX, &quantidade)) return 0;
    puts("Para cada transicao, informe origem, simbolo e destino separadamente.");
    puts("Use um simbolo do alfabeto ou l (letra L minuscula para lambda).");
    for (i = 0; i < quantidade; i++) {
        printf("\nTransicao %d:\n", i+1);
        if (!inteiro("  Origem (numero): ", 0, n-1, &origem)) return 0;
        for (;;) {
            printf("  Simbolo (ou l para lambda): ");
            fflush(stdout);
            if (!linha(buf, sizeof buf)) return 0;
            buf[strcspn(buf, "\r\n")] = '\0';
            if (strlen(buf) == 1 && indice_simbolo(a, buf[0]) >= 0) break;
            puts("Simbolo fora do alfabeto. Use um simbolo cadastrado ou l.");
        }
        simbolo = indice_simbolo(a, buf[0]);
        if (!inteiro("  Destino (numero): ", 0, n-1, &destino)) return 0;
        D(a, simbolo, origem, destino) = 1;
    }
    return 1;
}

/* Le tokens limitados e valida inteiros antes de acessar as matrizes. */
static int numero_arquivo(FILE *f, int minimo, int maximo, int *valor) {
    char token[128], *fim;
    long x;
    if (fscanf(f, "%127s", token) != 1) return 0;
    errno = 0;
    x = strtol(token, &fim, 10);
    if (fim == token || *fim || errno == ERANGE || x < minimo || x > maximo)
        return 0;
    *valor = (int)x;
    return 1;
}

static int ler_arquivo(Automato *a) {
    char nome[1024], simbolo[128];
    int n, m, quantidade, estado, origem, destino, i, ch, ok = 0;
    FILE *f;
    printf("Nome ou caminho do arquivo: ");
    fflush(stdout);
    if (!linha(nome, sizeof nome)) return 0;
    nome[strcspn(nome, "\r\n")] = '\0';
    f = fopen(nome, "r");
    if (!f) { perror("Nao foi possivel abrir o arquivo"); return 0; }
    if (!numero_arquivo(f, 1, 100, &n) ||
        !numero_arquivo(f, 1, LIMITE_SIMBOLOS, &m) || !criar(a, n, m)) goto fim;
    for (i = 0; i < m; i++)
        if (fscanf(f, "%127s", simbolo) != 1 || !adicionar_simbolo(a, i, simbolo))
            goto fim;
    if (!numero_arquivo(f, 0, n-1, &a->inicial) ||
        !numero_arquivo(f, 0, n, &quantidade)) goto fim;
    for (i = 0; i < quantidade; i++) {
        if (!numero_arquivo(f, 0, n-1, &estado) || a->finais[estado]) goto fim;
        a->finais[estado] = 1;
    }
    if (!numero_arquivo(f, 0, INT_MAX, &quantidade)) goto fim;
    for (i = 0; i < quantidade; i++) {
        if (!numero_arquivo(f, 0, n-1, &origem) ||
            fscanf(f, "%127s", simbolo) != 1 ||
            strlen(simbolo) != 1 || indice_simbolo(a, simbolo[0]) < 0 ||
            !numero_arquivo(f, 0, n-1, &destino)) goto fim;
        D(a, indice_simbolo(a, simbolo[0]),
          origem, destino) = 1;
    }
    while ((ch = fgetc(f)) != EOF)
        if (!isspace((unsigned char)ch)) goto fim;
    ok = !ferror(f);
fim:
    fclose(f);
    if (!ok) puts("Arquivo invalido: confira estados, alfabeto, finais e transicoes. Limite: 100 estados.");
    return ok;
}

static void conjunto(const unsigned char *estados, int n) {
    int i, primeiro = 1;
    printf("{");
    for (i = 0; i < n; i++) if (estados[i]) {
        printf("%sq%d", primeiro ? "" : ", ", i);
        primeiro = 0;
    }
    printf("}");
}

static void exibir(const Automato *a, int mostrar_lambda) {
    int i, s;
    printf("Quantidade de estados: %d\nEstados: {", a->n);
    for (i = 0; i < a->n; i++) printf("%sq%d", i ? ", " : "", i);
    printf("}\nAlfabeto: {");
    for (s = 0; s < a->m; s++) printf("%s%c", s ? ", " : "", a->alfabeto[s]);
    printf("}\nEstado inicial: q%d\nEstados finais: ", a->inicial);
    conjunto(a->finais, a->n);
    puts("\nTransicoes ({} significa conjunto vazio):");
    for (i = 0; i < a->n; i++) {
        for (s = 0; s < a->m; s++) {
            printf("  delta(q%d, %c) = ", i, a->alfabeto[s]);
            conjunto(&D(a, s, i, 0), a->n);
            putchar('\n');
        }
        if (mostrar_lambda) {
            printf("  delta(q%d, lambda) = ", i);
            conjunto(&D(a, a->m, i, 0), a->n);
            putchar('\n');
        }
    }
}

static int transformar(const Automato *entrada, Automato *saida) {
    int n = entrada->n, i, j, k, s, p, r;
    unsigned char *fecho, *apos_simbolo;
    if (!criar(saida, n, entrada->m)) return 0;
    fecho = calloc((size_t)n * n, 1);
    apos_simbolo = calloc((size_t)n, 1);
    if (!fecho || !apos_simbolo) {
        puts("Memoria insuficiente para calcular os fechos.");
        free(fecho);
        free(apos_simbolo);
        liberar(saida);
        return 0;
    }
    saida->inicial = entrada->inicial;
    memcpy(saida->alfabeto, entrada->alfabeto, (size_t)entrada->m);
    /* Fecho-lambda: inclui o proprio estado e todos os destinos via lambda. */
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++) C(i,j) = (i == j) || D(entrada,entrada->m,i,j);
    /* Fecho transitivo de Warshall: tambem resolve ciclos lambda. */
    for (k = 0; k < n; k++)
        for (i = 0; i < n; i++) if (C(i,k))
            for (j = 0; j < n; j++) if (C(k,j)) C(i,j) = 1;

    for (i = 0; i < n; i++) {
        for (p = 0; p < n; p++)
            if (C(i,p) && entrada->finais[p]) saida->finais[i] = 1;
        for (s = 0; s < entrada->m; s++) {
            memset(apos_simbolo, 0, (size_t)n);
            /* Primeiro: mover com o simbolo atual a partir do fecho de qi. */
            for (p = 0; p < n; p++) if (C(i,p))
                for (r = 0; r < n; r++)
                    if (D(entrada,s,p,r)) apos_simbolo[r] = 1;
            /* Depois: incluir o fecho de cada destino encontrado. */
            for (r = 0; r < n; r++) if (apos_simbolo[r])
                for (j = 0; j < n; j++) if (C(r,j)) D(saida,s,i,j) = 1;
        }
    }
    free(fecho);
    free(apos_simbolo);
    return 1;
}

/* Ponto de entrada para a opcao correspondente no menu do grupo. */
void transformar_afn_lambda_para_afn(void) {
    Automato entrada = {0}, convertido = {0};
    int opcao, ok;
    puts("\n=== AFN LAMBDA PARA AFN ===");
    puts("1 - Digitar\n2 - Ler arquivo .txt\n0 - Voltar");
    if (!inteiro("Opcao: ", 0, 2, &opcao) || opcao == 0) return;
    ok = opcao == 1 ? cadastrar(&entrada) : ler_arquivo(&entrada);
    if (ok && transformar(&entrada, &convertido)) {
        puts("\n--- AFN original (com lambda) ---");
        exibir(&entrada, 1);
        puts("\n--- AFN resultante (sem lambda) ---");
        exibir(&convertido, 0);
    }
    liberar(&entrada);
    liberar(&convertido);
    printf("\nPressione ENTER para continuar...");
    { char pausa[256]; (void)linha(pausa, sizeof pausa); }
}
