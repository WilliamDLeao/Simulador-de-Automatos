#include "json_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int ler_str(const char **p, char *out) {
  while (**p && **p != '"')
    (*p)++;
  if (!**p)
    return 0;
  (*p)++;
  while (**p && **p != '"') {
    *out++ = *(*p)++;
  }
  *out = '\0';
  if (**p == '"')
    (*p)++;
  return 1;
}

static int estado_id(const char *s) {
  return (s[0] == 'Q' || s[0] == 'q') ? atoi(s + 1) : atoi(s);
}

static void parse_secao(const char *txt, AutomatoJson *a) {
  char s[64];
  const char *p;

  p = strstr(txt, "\"alfabeto\"");
  if (p) {
    while (*p && *p != '[')
      p++;
    while (*p && *p != ']') {
      if (ler_str(&p, s) && s[0])
        a->alfabeto[a->m++] = s[0];
      while (*p && *p != ',' && *p != ']')
        p++;
      if (*p == ',')
        p++;
    }
  }

  p = strstr(txt, "\"estados\"");
  if (p) {
    while (*p && *p != '[')
      p++;
    while (*p && *p != ']') {
      if (ler_str(&p, s)) {
        strncpy(a->nomes_estados[a->n], s, JSON_MAX_NOME_ESTADO - 1);
        a->n++;
      }
      while (*p && *p != ',' && *p != ']')
        p++;
      if (*p == ',')
        p++;
    }
  }

  p = strstr(txt, "\"estados iniciais\"");
  if (p) {
    while (*p && *p != '[')
      p++;
    while (*p && *p != ']') {
      if (ler_str(&p, s))
        a->iniciais[a->qtd_iniciais++] = estado_id(s);
      while (*p && *p != ',' && *p != ']')
        p++;
      if (*p == ',')
        p++;
    }
  }

  p = strstr(txt, "\"estados finais\"");
  if (p) {
    while (*p && *p != '[')
      p++;
    while (*p && *p != ']') {
      if (ler_str(&p, s))
        a->finais[a->qtd_finais++] = estado_id(s);
      while (*p && *p != ',' && *p != ']')
        p++;
      if (*p == ',')
        p++;
    }
  }

  p = strstr(txt, "\"transi");
  if (p) {
    while (*p && *p != '[')
      p++;
    p++;
    while (*p && *p != ']') {
      while (*p && *p != '[' && *p != ']')
        p++;
      if (*p != '[')
        break;
      p++;
      char o[64] = "", d[64] = "", sim[64] = "";
      ler_str(&p, o);
      ler_str(&p, d);
      ler_str(&p, sim);
      while (*p && *p != ']')
        p++;
      if (*p == ']')
        p++;

      int idx = a->qtd_transicoes++;
      a->transicoes[idx].origem = estado_id(o);
      a->transicoes[idx].destino = estado_id(d);
      a->transicoes[idx].simbolo = sim[0];
      a->transicoes[idx].is_lambda =
          (sim[0] == 'l' || sim[0] == 'L' || !sim[0]);
    }
  }
}

static char *extrair_objeto(const char *inicio) {
  const char *p = strchr(inicio, '{');
  if (!p)
    return NULL;
  int depth = 1;
  const char *start = p;
  p++;
  while (*p && depth > 0) {
    if (*p == '{')
      depth++;
    else if (*p == '}')
      depth--;
    p++;
  }
  size_t len = (size_t)(p - start);
  char *bloco = malloc(len + 1);
  if (!bloco)
    return NULL;
  memcpy(bloco, start, len);
  bloco[len] = '\0';
  return bloco;
}

int lerArquivoJson(const char *caminho, ArquivoJsonAutomatos *res) {
  memset(res, 0, sizeof(*res));
  FILE *f = fopen(caminho, "rb");
  if (!f) {
    printf("Nao foi possivel abrir '%s'.\n", caminho);
    return 0;
  }
  fseek(f, 0, SEEK_END);
  long tam = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (tam <= 0) {
    fclose(f);
    return 0;
  }

  char *txt = malloc((size_t)tam + 1);
  if (!txt) {
    fclose(f);
    return 0;
  }
  fread(txt, 1, (size_t)tam, f);
  txt[tam] = '\0';
  fclose(f);

  const char *p1 = strstr(txt, "aut\xc3\xb4mato1");
  if (!p1)
    p1 = strstr(txt, "automato1");
  const char *p2 = strstr(txt, "aut\xc3\xb4mato2");
  if (!p2)
    p2 = strstr(txt, "automato2");

  if (p1) {
    char *b1 = extrair_objeto(p1);
    if (b1) {
      strcpy(res->a1.nome, "autômato1");
      parse_secao(b1, &res->a1);
      res->qtd_automatos = 1;
      free(b1);
    }
  }
  if (p2) {
    char *b2 = extrair_objeto(p2);
    if (b2) {
      strcpy(res->a2.nome, "autômato2");
      parse_secao(b2, &res->a2);
      res->qtd_automatos = 2;
      free(b2);
    }
  }
  if (!p1 && !p2) {
    strcpy(res->a1.nome, "autômato1");
    parse_secao(txt, &res->a1);
    res->qtd_automatos = 1;
  }

  free(txt);
  return res->qtd_automatos > 0;
}

int automatoJsonParaAFD(const AutomatoJson *aj, AFD *afd) {
  if (!aj || !afd)
    return 0;
  afd->n = aj->n;
  afd->m = aj->m;
  memcpy(afd->alfabeto, aj->alfabeto, aj->m);
  afd->inicial = aj->qtd_iniciais ? aj->iniciais[0] : 0;
  for (int i = 0; i < afd->n; i++) {
    afd->final[i] = 0;
    for (int s = 0; s < afd->m; s++)
      afd->delta[i][s] = -1;
  }
  for (int i = 0; i < aj->qtd_finais; i++)
    afd->final[aj->finais[i]] = 1;
  for (int t = 0; t < aj->qtd_transicoes; t++) {
    for (int s = 0; s < afd->m; s++) {
      if (afd->alfabeto[s] == aj->transicoes[t].simbolo) {
        afd->delta[aj->transicoes[t].origem][s] = aj->transicoes[t].destino;
        break;
      }
    }
  }
  return 1;
}

int automatoJsonParaAFN(const AutomatoJson *aj, AFN *afn) {
  if (!aj || !afn)
    return 0;
  afn->n = aj->n;
  afn->m = aj->m;
  memcpy(afn->alfabeto, aj->alfabeto, aj->m);
  afn->inicial = aj->qtd_iniciais ? aj->iniciais[0] : 0;
  afn->finais_mask = 0;
  for (int i = 0; i < MAX_ESTADOS; i++) {
    afn->final[i] = 0;
    for (int s = 0; s < MAX_SIMBOLOS; s++)
      afn->trans[i][s] = 0;
  }
  for (int i = 0; i < aj->qtd_finais; i++) {
    afn->final[aj->finais[i]] = 1;
    afn->finais_mask |= (1ULL << aj->finais[i]);
  }
  for (int t = 0; t < aj->qtd_transicoes; t++) {
    for (int s = 0; s < afn->m; s++) {
      if (afn->alfabeto[s] == aj->transicoes[t].simbolo) {
        afn->trans[aj->transicoes[t].origem][s] |=
            (1ULL << aj->transicoes[t].destino);
        break;
      }
    }
  }
  return 1;
}

int automatoJsonParaAFNMultiplo(const AutomatoJson *aj, AFNMultiplo *afnm) {
  if (!aj || !afnm)
    return 0;
  afnm->n = aj->n;
  afnm->m = aj->m;
  memcpy(afnm->alfabeto, aj->alfabeto, aj->m);
  afnm->qtd_iniciais = aj->qtd_iniciais ? aj->qtd_iniciais : 1;
  afnm->iniciais_mask = 0;
  afnm->qtd_finais = aj->qtd_finais;
  afnm->finais_mask = 0;
  for (int i = 0; i < MAX_ESTADOS; i++) {
    afnm->inicial[i] = 0;
    afnm->final[i] = 0;
    afnm->trans_lambda[i] = 0;
    for (int s = 0; s < MAX_SIMBOLOS; s++)
      afnm->trans[i][s] = 0;
  }
  for (int i = 0; i < aj->qtd_iniciais; i++) {
    afnm->inicial[aj->iniciais[i]] = 1;
    afnm->iniciais_mask |= (1ULL << aj->iniciais[i]);
  }
  for (int i = 0; i < aj->qtd_finais; i++) {
    afnm->final[aj->finais[i]] = 1;
    afnm->finais_mask |= (1ULL << aj->finais[i]);
  }
  for (int t = 0; t < aj->qtd_transicoes; t++) {
    int o = aj->transicoes[t].origem, d = aj->transicoes[t].destino;
    if (aj->transicoes[t].is_lambda) {
      afnm->trans_lambda[o] |= (1ULL << d);
    } else {
      for (int s = 0; s < afnm->m; s++) {
        if (afnm->alfabeto[s] == aj->transicoes[t].simbolo) {
          afnm->trans[o][s] |= (1ULL << d);
          break;
        }
      }
    }
  }
  return 1;
}

static const AutomatoJson *selecionar(const ArquivoJsonAutomatos *arq,
                                      int idx) {
  if (arq->qtd_automatos <= 1 || idx == 1)
    return &arq->a1;
  if (idx == 2)
    return &arq->a2;
  printf("\nArquivo contem 2 automatos. Escolha (1 ou 2) [1]: ");
  char b[16] = {0};
  if (fgets(b, sizeof(b), stdin) && (b[0] == '\n' || b[0] == '\r'))
    fgets(b, sizeof(b), stdin);
  return (b[0] == '2') ? &arq->a2 : &arq->a1;
}

int lerAFDJson(const char *cam, AFD *a, int idx) {
  ArquivoJsonAutomatos r;
  return lerArquivoJson(cam, &r) &&
         automatoJsonParaAFD(selecionar(&r, idx), a);
}

int lerAFNJson(const char *cam, AFN *a, int idx) {
  ArquivoJsonAutomatos r;
  return lerArquivoJson(cam, &r) &&
         automatoJsonParaAFN(selecionar(&r, idx), a);
}

int lerAFNMultiploJson(const char *cam, AFNMultiplo *a, int idx) {
  ArquivoJsonAutomatos r;
  return lerArquivoJson(cam, &r) &&
         automatoJsonParaAFNMultiplo(selecionar(&r, idx), a);
}

int lerDoisAFDsJson(const char *cam, AFD *a1, AFD *a2) {
  ArquivoJsonAutomatos r;
  if (!lerArquivoJson(cam, &r) || r.qtd_automatos < 2) {
    printf("O arquivo precisa ter 2 automatos (automato1 e automato2).\n");
    return 0;
  }
  return automatoJsonParaAFD(&r.a1, a1) && automatoJsonParaAFD(&r.a2, a2);
}

int lerDoisAFNsJson(const char *cam, AFN *a1, AFN *a2) {
  ArquivoJsonAutomatos r;
  if (!lerArquivoJson(cam, &r) || r.qtd_automatos < 2) {
    printf("O arquivo precisa ter 2 automatos (automato1 e automato2).\n");
    return 0;
  }
  return automatoJsonParaAFN(&r.a1, a1) && automatoJsonParaAFN(&r.a2, a2);
}
