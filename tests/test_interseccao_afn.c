#include "interseccao_automatos.h"
#include "complemento_afn.h"
#include "minimizar_afd.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Oraculo independente: simula conjuntos de estados, sem determinizar. */
static int aceitaAFN(const AFN *a, const char *w) {
  uint64_t atuais = UINT64_C(1) << a->inicial;
  for (; *w; ++w) {
    int s = 0;
    while (s < a->m && a->alfabeto[s] != *w) ++s;
    if (s == a->m) return 0;
    uint64_t proximos = 0;
    for (int q = 0; q < a->n; ++q)
      if (atuais & (UINT64_C(1) << q)) proximos |= a->trans[q][s];
    atuais = proximos;
  }
  return !!(atuais & a->finais_mask);
}

static unsigned comparacoes;
static void conferir(const AFN *a, const AFN *b) {
  AFD r, minimo;
  AFN equivalente = {0};
  assert(intersectarAFNs(a, b, &r));
  assert(minimizarAFD(&r, &minimo));
  int tem_afn = converterAFDparaAFN(&r, &equivalente);
  for (int n = 0; n <= 8; ++n) {
    for (unsigned bits = 0; bits < (1u << n); ++bits) {
      char w[9];
      for (int i = 0; i < n; ++i) w[i] = bits & (1u << i) ? 'b' : 'a';
      w[n] = 0;
      int esperado = aceitaAFN(a, w) && aceitaAFN(b, w);
      assert((simularPalavraAFD(&r, w) == 1) == esperado);
      assert((simularPalavraAFD(&minimo, w) == 1) == esperado);
      if (tem_afn) assert(aceitaAFN(&equivalente, w) == esperado);
      ++comparacoes;
    }
  }
}

int main(void) {
  AFN a = {0}, b = {0};
  AFD r = {0}, anterior;
  assert(lerAFNArquivo("exemplo_interseccao_afn_m1.txt", &a));
  assert(lerAFNArquivo("exemplo_interseccao_afn_m2.txt", &b));
  conferir(&a, &b);
  assert(intersectarAFNs(&a, &b, &r));
  assert(simularPalavraAFD(&r, "aab") == 1);
  assert(simularPalavraAFD(&r, "ab") == 0);
  assert(simularPalavraAFD(&r, "aa") == 0);
  /* AFN + AFD, estados sem transicoes e finais vazios. */
  assert(lerAFNArquivo("exemplo_uniao_m2.txt", &b));
  conferir(&a, &b);
  memset(b.final, 0, sizeof b.final); b.finais_mask = 0;
  conferir(&a, &b);
  /* Estado 63 e mascara de 64 bits: nao deslocar por 64. */
  memset(&a, 0, sizeof a); memset(&b, 0, sizeof b);
  a.n = 64; a.m = 1; a.alfabeto[0] = 'a'; a.inicial = 63;
  a.final[63] = 1; a.finais_mask = UINT64_C(1) << 63;
  a.trans[63][0] = UINT64_C(1) << 63;
  b = a; conferir(&a, &b);
  b.alfabeto[0] = 'b'; conferir(&a, &b);
  /* Saida intacta em entrada invalida. */
  anterior = r;
  a.n = 65; assert(!intersectarAFNs(&a, &b, &r));
  assert(memcmp(&r, &anterior, sizeof r) == 0);
  a = b; a.finais_mask = 0; assert(!intersectarAFNs(&a, &b, &r));
  a = b; a.m = 2; a.alfabeto[1] = 'b'; assert(!intersectarAFNs(&a, &b, &r));
  a = b; a.n = 2; a.inicial = 0; a.finais_mask = 0; a.trans[0][0] = 4;
  assert(!intersectarAFNs(&a, &b, &r));
  assert(!intersectarAFNs(NULL, &b, &r));
  assert(!intersectarAFNs(&b, &b, NULL));
  /* AFN que memoriza o setimo simbolo a partir do fim: >100 subconjuntos. */
  memset(&a, 0, sizeof a);
  a.n = 8; a.m = 2; a.alfabeto[0] = 'a'; a.alfabeto[1] = 'b';
  a.final[7] = 1; a.finais_mask = 128;
  a.trans[0][0] = 3; a.trans[0][1] = 1;
  for (int q = 1; q < 7; ++q) a.trans[q][0] = a.trans[q][1] = UINT64_C(1) << (q + 1);
  assert(!intersectarAFNs(&a, &a, &r));
  assert(memcmp(&r, &anterior, sizeof r) == 0);
  /* Pares aleatorios de AFNs com ramificacao real e alfabetos reordenados. */
  unsigned seed = 91;
  for (int t = 0; t < 100; ++t) {
    memset(&a, 0, sizeof a); memset(&b, 0, sizeof b);
    AFN *lista[] = {&a, &b};
    for (int j = 0; j < 2; ++j) {
      AFN *x = lista[j]; x->n = 3; x->m = 2; x->inicial = t % 3;
      x->alfabeto[0] = j ? 'b' : 'a'; x->alfabeto[1] = j ? 'a' : 'b';
      for (int p = 0; p < 3; ++p) {
        seed = seed * 1664525u + 1013904223u;
        x->final[p] = (seed >> 16) & 1;
        if (x->final[p]) x->finais_mask |= UINT64_C(1) << p;
        for (int s = 0; s < 2; ++s) {
          seed = seed * 1664525u + 1013904223u;
          x->trans[p][s] = (seed >> 16) & 7;
        }
      }
    }
    conferir(&a, &b);
  }
  printf("OK: %u palavras em AFNs; produto, minimizado e AFN equivalente conferidos.\n", comparacoes);
  return 0;
}
