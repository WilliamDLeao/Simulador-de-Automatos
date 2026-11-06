#include "interseccao_automatos.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int aceita(const AFD *a, const char *w) {
  int p = a->inicial;
  for (; *w; ++w) {
    int s;
    for (s = 0; s < a->m && a->alfabeto[s] != *w; ++s) {}
    if (s == a->m || a->delta[p][s] < 0) return 0;
    p = a->delta[p][s];
  }
  return !!a->final[p];
}

static unsigned verificadas;
static void comparar(const AFD *a, const AFD *b) {
  AFD r;
  assert(intersectarAFDs(a, b, &r));
  for (int n = 0; n <= 8; ++n) {
    for (unsigned bits = 0; bits < (1u << n); ++bits) {
      char w[9];
      for (int i = 0; i < n; ++i) w[i] = (bits & (1u << i)) ? 'b' : 'a';
      w[n] = 0;
      assert(aceita(&r, w) == (aceita(a, w) && aceita(b, w)));
      ++verificadas;
    }
  }
}

static AFD ciclo(int n) {
  AFD a = {0};
  a.n = n; a.m = 1; a.alfabeto[0] = 'a'; a.final[0] = 1;
  for (int p = 0; p < n; ++p) a.delta[p][0] = (p + 1) % n;
  return a;
}

int main(void) {
  AFD a, b, r, antes, copia;
  /* M1 aceita quantidade par de a; b preserva o estado. */
  a = ciclo(2);
  a.m = 2;
  a.alfabeto[1] = 'b';
  for (int q = 0; q < a.n; ++q) a.delta[q][1] = q;
  assert(lerAFDArquivo("exemplo_uniao_m2.txt", &b));
  comparar(&a, &b);
  assert(intersectarAFDs(&a, &b, &r));
  assert(r.n == 4 && r.inicial == 0 && r.final[2]);
  assert(aceita(&r, "aab") && !aceita(&r, "ab"));
  copia = a;
  assert(intersectarAFDs(&copia, &b, &copia));
  assert(memcmp(&copia, &r, sizeof r) == 0);
  /* Ordem diferente das colunas, preservando o significado. */
  b.alfabeto[0] = 'b'; b.alfabeto[1] = 'a';
  for (int p = 0; p < b.n; ++p) {
    int tmp = b.delta[p][0]; b.delta[p][0] = b.delta[p][1]; b.delta[p][1] = tmp;
  }
  comparar(&a, &b);
  /* Transicoes parciais e inicial diferente de zero. */
  a.delta[1][0] = -1; a.inicial = 1;
  comparar(&a, &b);
  /* Vazia aceita; alfabetos disjuntos; sem finais. */
  a = ciclo(1); b = ciclo(1);
  comparar(&a, &b);
  assert(intersectarAFDs(&a, &b, &r) && aceita(&r, ""));
  b.alfabeto[0] = 'b'; comparar(&a, &b);
  b.final[0] = 0; comparar(&a, &b);
  /* Produto com exatamente 100 estados; depois 110, que deve falhar. */
  a = ciclo(10); b = ciclo(20);
  assert(intersectarAFDs(&a, &b, &r) && r.n == 20);
  a = ciclo(4); b = ciclo(25);
  assert(intersectarAFDs(&a, &b, &r) && r.n == MAX_ESTADOS);
  antes = r;
  a = ciclo(10); b = ciclo(11);
  assert(!intersectarAFDs(&a, &b, &r));
  assert(memcmp(&antes, &r, sizeof r) == 0);
  a.n = 0; assert(!intersectarAFDs(&a, &b, &r));
  a = ciclo(1); a.delta[0][0] = 1;
  assert(!intersectarAFDs(&a, &b, &r));
  a = ciclo(1); a.m = 2; a.alfabeto[1] = 'a';
  assert(!intersectarAFDs(&a, &b, &r));
  assert(!intersectarAFDs(NULL, &b, &r));
  assert(!intersectarAFDs(&b, &b, NULL));
  a = ciclo(1); a.m = MAX_SIMBOLOS;
  for (int s = 0; s < a.m; ++s) a.alfabeto[s] = (char)('a' + s);
  b = ciclo(1); b.alfabeto[0] = '0';
  assert(!intersectarAFDs(&a, &b, &r));
  /* 100 pares pequenos pseudoaleatorios, incluindo ausencia de transicoes. */
  unsigned seed = 7;
  for (int t = 0; t < 100; ++t) {
    a = ciclo(3); b = ciclo(4);
    AFD *entradas[] = {&a, &b};
    for (int j = 0; j < 2; ++j) {
      AFD *x = entradas[j]; x->m = 2; x->alfabeto[1] = 'b';
      x->inicial = t % x->n;
      for (int p = 0; p < x->n; ++p) {
        seed = seed * 1664525u + 1013904223u;
        x->final[p] = (int)((seed >> 16) & 1u);
        for (int s = 0; s < x->m; ++s) {
          seed = seed * 1664525u + 1013904223u;
          x->delta[p][s] = (int)((seed >> 16) % (unsigned)(x->n + 1)) - 1;
        }
      }
    }
    comparar(&a, &b);
  }
  printf("OK: %u comparacoes de palavras e casos de fronteira.\n", verificadas);
  return 0;
}
