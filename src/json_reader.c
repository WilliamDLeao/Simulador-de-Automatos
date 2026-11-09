#include "json_reader.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Comparação de strings sem diferenciação de maiúsculas/minúsculas (ASCII) */
static int str_case_eq(const char *s1, const char *s2) {
  if (!s1 || !s2)
    return 0;
  while (*s1 && *s2) {
    if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2))
      return 0;
    s1++;
    s2++;
  }
  return *s1 == *s2;
}

/* Remove espaços e converte caracteres acentuados comuns em UTF-8 para ASCII
 * minúsculo */
static void normalizarChave(const char *src, char *dst, size_t max_dst) {
  size_t i = 0, j = 0;
  while (src[i] && j + 1 < max_dst) {
    unsigned char c = (unsigned char)src[i];
    if (c == ' ' || c == '_' || c == '-' || c == '\t' || c == '\r' ||
        c == '\n') {
      i++;
      continue;
    }
    /* Tratamento de sequências UTF-8 de 2 bytes com acentos comuns em português
     */
    if (c == 0xC3 && src[i + 1] != '\0') {
      unsigned char c2 = (unsigned char)src[i + 1];
      switch (c2) {
      case 0xB4: /* ô */
      case 0x94: /* Ô */
      case 0xB5: /* õ */
      case 0x95: /* Õ */
      case 0xB3: /* ó */
      case 0x93: /* Ó */
        dst[j++] = 'o';
        break;
      case 0xA7: /* ç */
      case 0x87: /* Ç */
        dst[j++] = 'c';
        break;
      case 0xA3: /* ã */
      case 0x83: /* Ã */
      case 0xA1: /* á */
      case 0x81: /* Á */
      case 0xA0: /* à */
      case 0x80: /* À */
      case 0xA2: /* â */
      case 0x82: /* Â */
        dst[j++] = 'a';
        break;
      case 0xA9: /* é */
      case 0x89: /* É */
      case 0xAA: /* ê */
      case 0x8A: /* Ê */
        dst[j++] = 'e';
        break;
      case 0xAD: /* í */
      case 0x8D: /* Í */
        dst[j++] = 'i';
        break;
      case 0xBA: /* ú */
      case 0x9A: /* Ú */
        dst[j++] = 'u';
        break;
      default:
        dst[j++] = '?';
        break;
      }
      i += 2;
      continue;
    }
    dst[j++] = (char)tolower(c);
    i++;
  }
  dst[j] = '\0';
}

/* Tipos de tokens do parser JSON */
typedef enum {
  TOK_EOF,
  TOK_LBRACE,   /* { */
  TOK_RBRACE,   /* } */
  TOK_LBRACKET, /* [ */
  TOK_RBRACKET, /* ] */
  TOK_COLON,    /* : */
  TOK_COMMA,    /* , */
  TOK_STRING,   /* "..." */
  TOK_NUMBER,   /* 0..9 */
  TOK_OTHER
} TokenType;

typedef struct {
  const char *src;
  size_t len;
  size_t pos;
  TokenType tipo;
  char str_val[256];
} JsonParser;

static TokenType proximoToken(JsonParser *p) {
  while (p->pos < p->len) {
    char c = p->src[p->pos];
    if (isspace((unsigned char)c)) {
      p->pos++;
      continue;
    }
    /* Ignora comentários // */
    if (c == '/' && p->pos + 1 < p->len && p->src[p->pos + 1] == '/') {
      p->pos += 2;
      while (p->pos < p->len && p->src[p->pos] != '\n')
        p->pos++;
      continue;
    }
    /* Ignora comentários em bloco */
    if (c == '/' && p->pos + 1 < p->len && p->src[p->pos + 1] == '*') {
      p->pos += 2;
      while (p->pos + 1 < p->len &&
             !(p->src[p->pos] == '*' && p->src[p->pos + 1] == '/'))
        p->pos++;
      if (p->pos + 1 < p->len)
        p->pos += 2;
      continue;
    }
    break;
  }

  if (p->pos >= p->len) {
    p->tipo = TOK_EOF;
    p->str_val[0] = '\0';
    return TOK_EOF;
  }

  char c = p->src[p->pos];
  if (c == '{') {
    p->pos++;
    p->tipo = TOK_LBRACE;
    return TOK_LBRACE;
  }
  if (c == '}') {
    p->pos++;
    p->tipo = TOK_RBRACE;
    return TOK_RBRACE;
  }
  if (c == '[') {
    p->pos++;
    p->tipo = TOK_LBRACKET;
    return TOK_LBRACKET;
  }
  if (c == ']') {
    p->pos++;
    p->tipo = TOK_RBRACKET;
    return TOK_RBRACKET;
  }
  if (c == ':') {
    p->pos++;
    p->tipo = TOK_COLON;
    return TOK_COLON;
  }
  if (c == ',') {
    p->pos++;
    p->tipo = TOK_COMMA;
    return TOK_COMMA;
  }

  /* String */
  if (c == '"') {
    p->pos++;
    size_t out_len = 0;
    while (p->pos < p->len && p->src[p->pos] != '"') {
      if (p->src[p->pos] == '\\' && p->pos + 1 < p->len) {
        p->pos++;
        char esc = p->src[p->pos++];
        if (esc == 'n')
          esc = '\n';
        else if (esc == 't')
          esc = '\t';
        else if (esc == 'r')
          esc = '\r';
        if (out_len + 1 < sizeof(p->str_val))
          p->str_val[out_len++] = esc;
      } else {
        if (out_len + 1 < sizeof(p->str_val))
          p->str_val[out_len++] = p->src[p->pos++];
        else
          p->pos++;
      }
    }
    if (p->pos < p->len && p->src[p->pos] == '"')
      p->pos++;
    p->str_val[out_len] = '\0';
    p->tipo = TOK_STRING;
    return TOK_STRING;
  }

  /* Número ou identificador sem aspas */
  if (isdigit((unsigned char)c) || c == '-') {
    size_t out_len = 0;
    while (p->pos < p->len &&
           (isdigit((unsigned char)p->src[p->pos]) || p->src[p->pos] == '.' ||
            p->src[p->pos] == '-')) {
      if (out_len + 1 < sizeof(p->str_val))
        p->str_val[out_len++] = p->src[p->pos++];
      else
        p->pos++;
    }
    p->str_val[out_len] = '\0';
    p->tipo = TOK_NUMBER;
    return TOK_NUMBER;
  }

  p->pos++;
  p->tipo = TOK_OTHER;
  return TOK_OTHER;
}

static void pularValor(JsonParser *p) {
  if (p->tipo == TOK_LBRACE) {
    int depth = 1;
    while (depth > 0 && proximoToken(p) != TOK_EOF) {
      if (p->tipo == TOK_LBRACE)
        depth++;
      else if (p->tipo == TOK_RBRACE)
        depth--;
    }
    proximoToken(p);
  } else if (p->tipo == TOK_LBRACKET) {
    int depth = 1;
    while (depth > 0 && proximoToken(p) != TOK_EOF) {
      if (p->tipo == TOK_LBRACKET)
        depth++;
      else if (p->tipo == TOK_RBRACKET)
        depth--;
    }
    proximoToken(p);
  } else {
    proximoToken(p);
  }
}

/* Busca o índice do estado pelo nome ou cadastra caso não exista */
static int obterOuCadastrarEstado(AutomatoJson *a, const char *nome) {
  if (!nome || !nome[0])
    return 0;

  /* 1) Busca nos estados já conhecidos */
  for (int i = 0; i < a->n; i++) {
    if (str_case_eq(a->nomes_estados[i], nome))
      return i;
  }

  /* 2) Se o nome for no formato Q<num> ou q<num> */
  if ((nome[0] == 'Q' || nome[0] == 'q') && isdigit((unsigned char)nome[1])) {
    int id = atoi(nome + 1);
    if (id >= 0 && id < JSON_MAX_ESTADOS) {
      if (id >= a->n) {
        for (int k = a->n; k <= id; k++) {
          snprintf(a->nomes_estados[k], sizeof(a->nomes_estados[k]), "Q%d", k);
        }
        a->n = id + 1;
      }
      return id;
    }
  }

  /* 3) Se for puramente número */
  int apenas_num = 1;
  for (int k = 0; nome[k]; k++) {
    if (!isdigit((unsigned char)nome[k])) {
      apenas_num = 0;
      break;
    }
  }
  if (apenas_num) {
    int id = atoi(nome);
    if (id >= 0 && id < JSON_MAX_ESTADOS) {
      if (id >= a->n) {
        for (int k = a->n; k <= id; k++) {
          snprintf(a->nomes_estados[k], sizeof(a->nomes_estados[k]), "Q%d", k);
        }
        a->n = id + 1;
      }
      return id;
    }
  }

  /* 4) Cadastra novo estado com o nome literal informado */
  if (a->n < JSON_MAX_ESTADOS) {
    int id = a->n++;
    strncpy(a->nomes_estados[id], nome, JSON_MAX_NOME_ESTADO - 1);
    a->nomes_estados[id][JSON_MAX_NOME_ESTADO - 1] = '\0';
    return id;
  }

  return 0;
}

static int ehSimboloAlfabeto(const AutomatoJson *a, const char *s) {
  if (!s || strlen(s) != 1)
    return 0;
  for (int i = 0; i < a->m; i++) {
    if (a->alfabeto[i] == s[0])
      return 1;
  }
  return 0;
}

static int ehSimboloLambda(const char *s) {
  if (!s)
    return 0;
  if (strcmp(s, "l") == 0 || strcmp(s, "L") == 0 || str_case_eq(s, "lambda") ||
      str_case_eq(s, "epsilon") || strcmp(s, "\xce\xbb") == 0 || /* λ */
      strcmp(s, "\xce\xb5") == 0 ||                              /* ε */
      strcmp(s, "-") == 0 || strcmp(s, "_") == 0 || s[0] == '\0') {
    return 1;
  }
  return 0;
}

/* Processa uma transição lida do JSON: suporta [origem, destino, simbolo] e
 * [origem, simbolo, destino] */
static void processarTransicaoJson(AutomatoJson *a, char elems[3][64],
                                   int count) {
  if (count < 2 || a->qtd_transicoes >=
                       (int)(sizeof(a->transicoes) / sizeof(a->transicoes[0])))
    return;

  int o = 0, d = 0, is_lambda = 0;
  char sim = 'l';

  if (count == 2) {
    /* [origem, destino] -> assume lambda */
    o = obterOuCadastrarEstado(a, elems[0]);
    d = obterOuCadastrarEstado(a, elems[1]);
    is_lambda = 1;
    sim = 'l';
  } else {
    /* count == 3 */
    int elem2_alf = ehSimboloAlfabeto(a, elems[1]);
    int elem3_alf = ehSimboloAlfabeto(a, elems[2]);
    int elem2_lam = ehSimboloLambda(elems[1]);
    int elem3_lam = ehSimboloLambda(elems[2]);

    if (elem3_alf || elem3_lam) {
      /* Formato padrão do exemplo_leitura.json: [origem, destino, simbolo] */
      o = obterOuCadastrarEstado(a, elems[0]);
      d = obterOuCadastrarEstado(a, elems[1]);
      if (elem3_lam) {
        is_lambda = 1;
        sim = 'l';
      } else {
        is_lambda = 0;
        sim = elems[2][0];
      }
    } else if (elem2_alf || elem2_lam) {
      /* Formato alternativo: [origem, simbolo, destino] */
      o = obterOuCadastrarEstado(a, elems[0]);
      d = obterOuCadastrarEstado(a, elems[2]);
      if (elem2_lam) {
        is_lambda = 1;
        sim = 'l';
      } else {
        is_lambda = 0;
        sim = elems[1][0];
      }
    } else {
      /* Caso não esteja explícito no alfabeto prévio: verifica qual parece
       * símbolo (1 caractere) */
      if (strlen(elems[2]) == 1 && strlen(elems[1]) > 1) {
        o = obterOuCadastrarEstado(a, elems[0]);
        d = obterOuCadastrarEstado(a, elems[1]);
        sim = elems[2][0];
        is_lambda = ehSimboloLambda(elems[2]);
      } else if (strlen(elems[1]) == 1 && strlen(elems[2]) > 1) {
        o = obterOuCadastrarEstado(a, elems[0]);
        d = obterOuCadastrarEstado(a, elems[2]);
        sim = elems[1][0];
        is_lambda = ehSimboloLambda(elems[1]);
      } else {
        /* Default: [origem, destino, simbolo] */
        o = obterOuCadastrarEstado(a, elems[0]);
        d = obterOuCadastrarEstado(a, elems[1]);
        sim = elems[2][0];
        is_lambda = ehSimboloLambda(elems[2]);
      }
      /* Adiciona ao alfabeto se não for lambda */
      if (!is_lambda && sim != '\0' &&
          !ehSimboloAlfabeto(a, (char[]){sim, '\0'})) {
        if (a->m < JSON_MAX_SIMBOLOS) {
          a->alfabeto[a->m++] = sim;
        }
      }
    }
  }

  a->transicoes[a->qtd_transicoes].origem = o;
  a->transicoes[a->qtd_transicoes].destino = d;
  a->transicoes[a->qtd_transicoes].simbolo = is_lambda ? 'l' : sim;
  a->transicoes[a->qtd_transicoes].is_lambda = is_lambda;
  a->qtd_transicoes++;
}

/* Faz o parse do objeto de um autômato */
static void parseObjetoAutomato(JsonParser *p, AutomatoJson *a) {
  if (p->tipo != TOK_LBRACE)
    return;
  proximoToken(p); /* Consome '{' */

  while (p->tipo != TOK_RBRACE && p->tipo != TOK_EOF) {
    if (p->tipo != TOK_STRING) {
      proximoToken(p);
      continue;
    }

    char chave[64];
    normalizarChave(p->str_val, chave, sizeof(chave));
    proximoToken(p);

    if (p->tipo != TOK_COLON) {
      pularValor(p);
      continue;
    }
    proximoToken(p); /* Consome ':' */

    if (strcmp(chave, "alfabeto") == 0 || strcmp(chave, "alphabet") == 0) {
      if (p->tipo == TOK_LBRACKET) {
        proximoToken(p);
        while (p->tipo != TOK_RBRACKET && p->tipo != TOK_EOF) {
          if (p->tipo == TOK_STRING) {
            if (p->str_val[0] != '\0' && a->m < JSON_MAX_SIMBOLOS) {
              int ja_tem = 0;
              for (int k = 0; k < a->m; k++) {
                if (a->alfabeto[k] == p->str_val[0]) {
                  ja_tem = 1;
                  break;
                }
              }
              if (!ja_tem) {
                a->alfabeto[a->m++] = p->str_val[0];
              }
            }
            proximoToken(p);
          } else if (p->tipo == TOK_COMMA) {
            proximoToken(p);
          } else {
            proximoToken(p);
          }
        }
        if (p->tipo == TOK_RBRACKET)
          proximoToken(p);
      } else {
        pularValor(p);
      }
    } else if (strcmp(chave, "estados") == 0 || strcmp(chave, "states") == 0) {
      if (p->tipo == TOK_LBRACKET) {
        proximoToken(p);
        while (p->tipo != TOK_RBRACKET && p->tipo != TOK_EOF) {
          if (p->tipo == TOK_STRING || p->tipo == TOK_NUMBER) {
            obterOuCadastrarEstado(a, p->str_val);
            proximoToken(p);
          } else if (p->tipo == TOK_COMMA) {
            proximoToken(p);
          } else {
            proximoToken(p);
          }
        }
        if (p->tipo == TOK_RBRACKET)
          proximoToken(p);
      } else {
        pularValor(p);
      }
    } else if (strcmp(chave, "estadosiniciais") == 0 ||
               strcmp(chave, "iniciais") == 0 ||
               strcmp(chave, "estadoinicial") == 0 ||
               strcmp(chave, "initialstates") == 0 ||
               strcmp(chave, "initial") == 0) {
      if (p->tipo == TOK_LBRACKET) {
        proximoToken(p);
        while (p->tipo != TOK_RBRACKET && p->tipo != TOK_EOF) {
          if (p->tipo == TOK_STRING || p->tipo == TOK_NUMBER) {
            int id = obterOuCadastrarEstado(a, p->str_val);
            if (a->qtd_iniciais < JSON_MAX_ESTADOS) {
              a->iniciais[a->qtd_iniciais++] = id;
            }
            proximoToken(p);
          } else if (p->tipo == TOK_COMMA) {
            proximoToken(p);
          } else {
            proximoToken(p);
          }
        }
        if (p->tipo == TOK_RBRACKET)
          proximoToken(p);
      } else if (p->tipo == TOK_STRING || p->tipo == TOK_NUMBER) {
        int id = obterOuCadastrarEstado(a, p->str_val);
        if (a->qtd_iniciais < JSON_MAX_ESTADOS) {
          a->iniciais[a->qtd_iniciais++] = id;
        }
        proximoToken(p);
      } else {
        pularValor(p);
      }
    } else if (strcmp(chave, "estadosfinais") == 0 ||
               strcmp(chave, "finais") == 0 ||
               strcmp(chave, "estadofinal") == 0 ||
               strcmp(chave, "finalstates") == 0 ||
               strcmp(chave, "final") == 0) {
      if (p->tipo == TOK_LBRACKET) {
        proximoToken(p);
        while (p->tipo != TOK_RBRACKET && p->tipo != TOK_EOF) {
          if (p->tipo == TOK_STRING || p->tipo == TOK_NUMBER) {
            int id = obterOuCadastrarEstado(a, p->str_val);
            if (a->qtd_finais < JSON_MAX_ESTADOS) {
              a->finais[a->qtd_finais++] = id;
            }
            proximoToken(p);
          } else if (p->tipo == TOK_COMMA) {
            proximoToken(p);
          } else {
            proximoToken(p);
          }
        }
        if (p->tipo == TOK_RBRACKET)
          proximoToken(p);
      } else if (p->tipo == TOK_STRING || p->tipo == TOK_NUMBER) {
        int id = obterOuCadastrarEstado(a, p->str_val);
        if (a->qtd_finais < JSON_MAX_ESTADOS) {
          a->finais[a->qtd_finais++] = id;
        }
        proximoToken(p);
      } else {
        pularValor(p);
      }
    } else if (strcmp(chave, "transicoes") == 0 ||
               strcmp(chave, "transicao") == 0 ||
               strcmp(chave, "transitions") == 0) {
      if (p->tipo == TOK_LBRACKET) {
        proximoToken(p);
        while (p->tipo != TOK_RBRACKET && p->tipo != TOK_EOF) {
          if (p->tipo == TOK_LBRACKET) {
            proximoToken(p);
            char elems[3][64] = {{0}};
            int count = 0;
            while (p->tipo != TOK_RBRACKET && p->tipo != TOK_EOF) {
              if (p->tipo == TOK_STRING || p->tipo == TOK_NUMBER) {
                if (count < 3) {
                  strncpy(elems[count++], p->str_val, 63);
                }
                proximoToken(p);
              } else if (p->tipo == TOK_COMMA) {
                proximoToken(p);
              } else {
                proximoToken(p);
              }
            }
            if (p->tipo == TOK_RBRACKET)
              proximoToken(p);
            processarTransicaoJson(a, elems, count);
          } else if (p->tipo == TOK_COMMA) {
            proximoToken(p);
          } else {
            proximoToken(p);
          }
        }
        if (p->tipo == TOK_RBRACKET)
          proximoToken(p);
      } else {
        pularValor(p);
      }
    } else {
      pularValor(p);
    }

    if (p->tipo == TOK_COMMA)
      proximoToken(p);
  }

  if (p->tipo == TOK_RBRACE)
    proximoToken(p);

  /* Se a quantidade de estados for 0, garante pelo menos 1 estado */
  if (a->n == 0) {
    a->n = 1;
    snprintf(a->nomes_estados[0], sizeof(a->nomes_estados[0]), "Q0");
  }
}

int lerArquivoJson(const char *caminho, ArquivoJsonAutomatos *res) {
  if (!caminho || !res)
    return 0;
  memset(res, 0, sizeof(*res));

  /* Remove eventuais aspas do caminho */
  char caminho_limpo[1024];
  const char *src = caminho;
  while (isspace((unsigned char)*src))
    src++;
  size_t len = strlen(src);
  while (len > 0 && isspace((unsigned char)src[len - 1]))
    len--;
  if (len >= 2 && ((src[0] == '"' && src[len - 1] == '"') ||
                   (src[0] == '\'' && src[len - 1] == '\''))) {
    src++;
    len -= 2;
  }
  if (len >= sizeof(caminho_limpo))
    len = sizeof(caminho_limpo) - 1;
  strncpy(caminho_limpo, src, len);
  caminho_limpo[len] = '\0';

  FILE *f = fopen(caminho_limpo, "rb");
  if (!f) {
    printf("Nao foi possivel abrir o arquivo JSON '%s'.\n", caminho_limpo);
    return 0;
  }

  fseek(f, 0, SEEK_END);
  long tam = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (tam <= 0 || tam > 10 * 1024 * 1024) { /* Máximo 10MB */
    printf("Tamanho de arquivo invalido (%ld bytes).\n", tam);
    fclose(f);
    return 0;
  }

  char *conteudo = (char *)malloc((size_t)tam + 1);
  if (!conteudo) {
    printf("Memoria insuficiente para carregar arquivo.\n");
    fclose(f);
    return 0;
  }

  size_t lidos = fread(conteudo, 1, (size_t)tam, f);
  conteudo[lidos] = '\0';
  fclose(f);

  JsonParser p;
  p.src = conteudo;
  p.len = lidos;
  p.pos = 0;
  proximoToken(&p);

  if (p.tipo != TOK_LBRACE) {
    printf("JSON invalido: raiz deve ser um objeto '{...}'.\n");
    free(conteudo);
    return 0;
  }
  proximoToken(&p); /* Consome '{' */

  int encontrou_chaves_raiz = 0;

  while (p.tipo != TOK_RBRACE && p.tipo != TOK_EOF) {
    if (p.tipo != TOK_STRING) {
      proximoToken(&p);
      continue;
    }

    char chave[64];
    normalizarChave(p.str_val, chave, sizeof(chave));
    proximoToken(&p);

    if (p.tipo != TOK_COLON) {
      pularValor(&p);
      continue;
    }
    proximoToken(&p);

    if (strcmp(chave, "automato1") == 0 || strcmp(chave, "m1") == 0 ||
        strcmp(chave, "af1") == 0 || strcmp(chave, "afd1") == 0 ||
        strcmp(chave, "afn1") == 0) {
      strncpy(res->a1.nome, "autômato1", sizeof(res->a1.nome) - 1);
      parseObjetoAutomato(&p, &res->a1);
      if (res->qtd_automatos < 1)
        res->qtd_automatos = 1;
      encontrou_chaves_raiz = 1;
    } else if (strcmp(chave, "automato2") == 0 || strcmp(chave, "m2") == 0 ||
               strcmp(chave, "af2") == 0 || strcmp(chave, "afd2") == 0 ||
               strcmp(chave, "afn2") == 0) {
      strncpy(res->a2.nome, "autômato2", sizeof(res->a2.nome) - 1);
      parseObjetoAutomato(&p, &res->a2);
      res->qtd_automatos = 2;
      encontrou_chaves_raiz = 1;
    } else if (strcmp(chave, "automato") == 0 || strcmp(chave, "af") == 0 ||
               strcmp(chave, "afd") == 0 || strcmp(chave, "afn") == 0) {
      strncpy(res->a1.nome, "autômato", sizeof(res->a1.nome) - 1);
      parseObjetoAutomato(&p, &res->a1);
      if (res->qtd_automatos < 1)
        res->qtd_automatos = 1;
      encontrou_chaves_raiz = 1;
    } else if (!encontrou_chaves_raiz &&
               (strcmp(chave, "alfabeto") == 0 ||
                strcmp(chave, "estados") == 0 ||
                strcmp(chave, "estadosiniciais") == 0 ||
                strcmp(chave, "estadosfinais") == 0 ||
                strcmp(chave, "transicoes") == 0)) {
      /* Caso onde a raiz é o próprio autômato diretamente (sem chave
       * 'autômato1') */
      free(conteudo);
      /* Reinicia o parser a partir da raiz como o autômato a1 */
      p.src = (const char *)malloc((size_t)tam + 1);
      if (!p.src)
        return 0;
      /* Reabre o arquivo para leitura direta */
      f = fopen(caminho_limpo, "rb");
      if (!f)
        return 0;
      lidos = fread((char *)p.src, 1, (size_t)tam, f);
      ((char *)p.src)[lidos] = '\0';
      fclose(f);
      p.len = lidos;
      p.pos = 0;
      proximoToken(&p);
      strncpy(res->a1.nome, "autômato", sizeof(res->a1.nome) - 1);
      parseObjetoAutomato(&p, &res->a1);
      res->qtd_automatos = 1;
      free((char *)p.src);
      return 1;
    } else {
      pularValor(&p);
    }

    if (p.tipo == TOK_COMMA)
      proximoToken(&p);
  }

  free(conteudo);

  if (res->qtd_automatos == 0 && res->a1.n > 0)
    res->qtd_automatos = 1;

  return (res->qtd_automatos > 0);
}

int automatoJsonParaAFD(const AutomatoJson *aj, AFD *afd) {
  if (!aj || !afd || aj->n < 1 || aj->n > MAX_ESTADOS) {
    printf("Erro: Automato possui %d estados (limite para AFD: %d).\n",
           aj ? aj->n : 0, MAX_ESTADOS);
    return 0;
  }

  afd->n = aj->n;
  afd->m = aj->m;
  for (int s = 0; s < aj->m; s++) {
    afd->alfabeto[s] = aj->alfabeto[s];
  }
  afd->inicial = (aj->qtd_iniciais > 0) ? aj->iniciais[0] : 0;

  for (int i = 0; i < afd->n; i++) {
    afd->final[i] = 0;
    for (int s = 0; s < afd->m; s++) {
      afd->delta[i][s] = -1;
    }
  }

  for (int i = 0; i < aj->qtd_finais; i++) {
    int f = aj->finais[i];
    if (f >= 0 && f < afd->n) {
      afd->final[f] = 1;
    }
  }

  for (int t = 0; t < aj->qtd_transicoes; t++) {
    int o = aj->transicoes[t].origem;
    int d = aj->transicoes[t].destino;
    char sim = aj->transicoes[t].simbolo;

    if (aj->transicoes[t].is_lambda) {
      printf("Aviso: Transicao lambda ignorada no AFD (q%d -> q%d).\n", o, d);
      continue;
    }

    int s = -1;
    for (int k = 0; k < afd->m; k++) {
      if (afd->alfabeto[k] == sim) {
        s = k;
        break;
      }
    }

    if (s < 0) {
      printf("Aviso: Simbolo '%c' nao pertence ao alfabeto do AFD.\n", sim);
      continue;
    }

    if (afd->delta[o][s] != -1 && afd->delta[o][s] != d) {
      printf("Aviso: Nao-determinismo detectado no AFD: q%d com '%c' tem mais "
             "de um destino (q%d e q%d).\n",
             o, sim, afd->delta[o][s], d);
    }
    afd->delta[o][s] = d;
  }

  return 1;
}

int automatoJsonParaAFN(const AutomatoJson *aj, AFN *afn) {
  if (!aj || !afn || aj->n < 1 || aj->n > MAX_AFN_ESTADOS) {
    printf("Erro: Automato possui %d estados (limite para AFN: %d).\n",
           aj ? aj->n : 0, MAX_AFN_ESTADOS);
    return 0;
  }

  afn->n = aj->n;
  afn->m = aj->m;
  for (int s = 0; s < aj->m; s++) {
    afn->alfabeto[s] = aj->alfabeto[s];
  }
  afn->inicial = (aj->qtd_iniciais > 0) ? aj->iniciais[0] : 0;
  afn->finais_mask = 0;

  for (int i = 0; i < MAX_ESTADOS; i++) {
    afn->final[i] = 0;
    for (int s = 0; s < MAX_SIMBOLOS; s++) {
      afn->trans[i][s] = 0;
    }
  }

  for (int i = 0; i < aj->qtd_finais; i++) {
    int f = aj->finais[i];
    if (f >= 0 && f < afn->n) {
      afn->final[f] = 1;
      afn->finais_mask |= (1ULL << f);
    }
  }

  for (int t = 0; t < aj->qtd_transicoes; t++) {
    int o = aj->transicoes[t].origem;
    int d = aj->transicoes[t].destino;
    char sim = aj->transicoes[t].simbolo;

    if (aj->transicoes[t].is_lambda) {
      printf(
          "Aviso: Transicao lambda ignorada no AFN sem lambda (q%d -> q%d).\n",
          o, d);
      continue;
    }

    int s = -1;
    for (int k = 0; k < afn->m; k++) {
      if (afn->alfabeto[k] == sim) {
        s = k;
        break;
      }
    }

    if (s >= 0 && o >= 0 && o < afn->n && d >= 0 && d < afn->n) {
      afn->trans[o][s] |= (1ULL << d);
    }
  }

  return 1;
}

int automatoJsonParaAFNMultiplo(const AutomatoJson *aj, AFNMultiplo *afnm) {
  if (!aj || !afnm || aj->n < 1 || aj->n > MAX_AFN_ME_ESTADOS) {
    printf("Erro: Automato possui %d estados (limite para AFN multiplos "
           "estados: %d).\n",
           aj ? aj->n : 0, MAX_AFN_ME_ESTADOS);
    return 0;
  }

  afnm->n = aj->n;
  afnm->m = aj->m;
  for (int s = 0; s < aj->m; s++) {
    afnm->alfabeto[s] = aj->alfabeto[s];
  }

  afnm->qtd_iniciais = 0;
  afnm->iniciais_mask = 0;
  afnm->qtd_finais = 0;
  afnm->finais_mask = 0;

  for (int i = 0; i < MAX_ESTADOS; i++) {
    afnm->inicial[i] = 0;
    afnm->final[i] = 0;
    afnm->trans_lambda[i] = 0;
    for (int s = 0; s < MAX_SIMBOLOS; s++) {
      afnm->trans[i][s] = 0;
    }
  }

  for (int i = 0; i < aj->qtd_iniciais; i++) {
    int ini = aj->iniciais[i];
    if (ini >= 0 && ini < afnm->n && !afnm->inicial[ini]) {
      afnm->inicial[ini] = 1;
      afnm->iniciais_mask |= (1ULL << ini);
      afnm->qtd_iniciais++;
    }
  }
  if (afnm->qtd_iniciais == 0) {
    afnm->inicial[0] = 1;
    afnm->iniciais_mask |= 1ULL;
    afnm->qtd_iniciais = 1;
  }

  for (int i = 0; i < aj->qtd_finais; i++) {
    int fin = aj->finais[i];
    if (fin >= 0 && fin < afnm->n && !afnm->final[fin]) {
      afnm->final[fin] = 1;
      afnm->finais_mask |= (1ULL << fin);
      afnm->qtd_finais++;
    }
  }

  for (int t = 0; t < aj->qtd_transicoes; t++) {
    int o = aj->transicoes[t].origem;
    int d = aj->transicoes[t].destino;
    char sim = aj->transicoes[t].simbolo;

    if (aj->transicoes[t].is_lambda || sim == 'l' || sim == 'L') {
      if (o >= 0 && o < afnm->n && d >= 0 && d < afnm->n) {
        afnm->trans_lambda[o] |= (1ULL << d);
      }
    } else {
      int s = -1;
      for (int k = 0; k < afnm->m; k++) {
        if (afnm->alfabeto[k] == sim) {
          s = k;
          break;
        }
      }
      if (s >= 0 && o >= 0 && o < afnm->n && d >= 0 && d < afnm->n) {
        afnm->trans[o][s] |= (1ULL << d);
      }
    }
  }

  return 1;
}

static const AutomatoJson *
selecionarAutomatoJson(const ArquivoJsonAutomatos *arq, const char *caminho,
                       int indice) {
  if (arq->qtd_automatos == 1)
    return &arq->a1;

  if (indice == 1)
    return &arq->a1;
  if (indice == 2)
    return &arq->a2;

  printf("\nO arquivo '%s' contem 2 automatos:\n", caminho);
  printf("  1 - %s (%d estados, %d simbolos)\n",
         arq->a1.nome[0] ? arq->a1.nome : "automato1", arq->a1.n, arq->a1.m);
  printf("  2 - %s (%d estados, %d simbolos)\n",
         arq->a2.nome[0] ? arq->a2.nome : "automato2", arq->a2.n, arq->a2.m);
  printf("Qual automato deseja utilizar? (1 ou 2) [padrao: 1]: ");

  char buf[64];
  int op = 1;
  if (fgets(buf, sizeof(buf), stdin) != NULL) {
    /* Se a linha for apenas '\n' ou '\r' (resquício de scanf anterior), lê
     * novamente */
    if (buf[0] == '\n' || buf[0] == '\r' || buf[0] == '\0') {
      if (fgets(buf, sizeof(buf), stdin) == NULL) {
        return &arq->a1;
      }
    }
    if (sscanf(buf, "%d", &op) != 1 || (op != 1 && op != 2))
      op = 1;
  }
  return (op == 2) ? &arq->a2 : &arq->a1;
}

int lerAFDJson(const char *caminho, AFD *a, int indice_automato) {
  ArquivoJsonAutomatos arq;
  if (!lerArquivoJson(caminho, &arq))
    return 0;
  const AutomatoJson *escolhido =
      selecionarAutomatoJson(&arq, caminho, indice_automato);
  return automatoJsonParaAFD(escolhido, a);
}

int lerAFNJson(const char *caminho, AFN *a, int indice_automato) {
  ArquivoJsonAutomatos arq;
  if (!lerArquivoJson(caminho, &arq))
    return 0;
  const AutomatoJson *escolhido =
      selecionarAutomatoJson(&arq, caminho, indice_automato);
  return automatoJsonParaAFN(escolhido, a);
}

int lerAFNMultiploJson(const char *caminho, AFNMultiplo *a,
                       int indice_automato) {
  ArquivoJsonAutomatos arq;
  if (!lerArquivoJson(caminho, &arq))
    return 0;
  const AutomatoJson *escolhido =
      selecionarAutomatoJson(&arq, caminho, indice_automato);
  return automatoJsonParaAFNMultiplo(escolhido, a);
}

int lerDoisAFDsJson(const char *caminho, AFD *a1, AFD *a2) {
  ArquivoJsonAutomatos arq;
  if (!lerArquivoJson(caminho, &arq))
    return 0;
  if (arq.qtd_automatos < 2) {
    printf("O arquivo '%s' contem apenas 1 automato. Esta funcionalidade "
           "requer 2 automatos (autômato1 e autômato2).\n",
           caminho);
    return 0;
  }
  if (!automatoJsonParaAFD(&arq.a1, a1))
    return 0;
  if (!automatoJsonParaAFD(&arq.a2, a2))
    return 0;
  return 1;
}

int lerDoisAFNsJson(const char *caminho, AFN *a1, AFN *a2) {
  ArquivoJsonAutomatos arq;
  if (!lerArquivoJson(caminho, &arq))
    return 0;
  if (arq.qtd_automatos < 2) {
    printf("O arquivo '%s' contem apenas 1 automato. Esta funcionalidade "
           "requer 2 automatos (autômato1 e autômato2).\n",
           caminho);
    return 0;
  }
  if (!automatoJsonParaAFN(&arq.a1, a1))
    return 0;
  if (!automatoJsonParaAFN(&arq.a2, a2))
    return 0;
  return 1;
}
