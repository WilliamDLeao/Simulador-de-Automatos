#ifndef AFD_H
#define AFD_H

#define MAX_ESTADOS 100
#define MAX_SIMBOLOS 26

/* Estrutura de AFD (pode ser usada pelos outros itens do trabalho).
   Estados: 0 .. n-1. delta[i][a] = -1 significa transicao inexistente. */
typedef struct {
  int n;                       /* quantidade de estados   */
  int m;                       /* tamanho do alfabeto     */
  char alfabeto[MAX_SIMBOLOS]; /* simbolos                */
  int inicial;                 /* estado inicial          */
  int final[MAX_ESTADOS];      /* final[i] = 1 se i final */
  int delta[MAX_ESTADOS][MAX_SIMBOLOS];
} AFD;

int lerAFD(AFD *a); /* le do teclado, retorna 1 se ok */
int lerAFDArquivo(const char *nome,
                  AFD *a); /* le de um .txt, retorna 1 se ok */
void imprimirAFD(const AFD *a);

#endif