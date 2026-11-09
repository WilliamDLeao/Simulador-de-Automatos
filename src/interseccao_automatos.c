#include "interseccao_automatos.h"
#include "complemento_afn.h"
#include "minimizar_afd.h"
#include "json_reader.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int valido(const AFD *a) {
  if (!a || a->n < 1 || a->n > MAX_ESTADOS || a->m < 1 ||
      a->m > MAX_SIMBOLOS || a->inicial < 0 || a->inicial >= a->n)
    return 0;
  for (int s = 0; s < a->m; ++s) {
    for (int t = 0; t < s; ++t)
      if (a->alfabeto[t] == a->alfabeto[s]) return 0;
    for (int p = 0; p < a->n; ++p)
      if (a->delta[p][s] < -1 || a->delta[p][s] >= a->n) return 0;
  }
  return 1;
}

/* -1 representa o poco implicito: rejeita e permanece no poco. */
static int passo(const AFD *a, int p, char c) {
  if (p < 0) return -1;
  for (int s = 0; s < a->m; ++s)
    if (a->alfabeto[s] == c) return a->delta[p][s];
  return -1;
}

int intersectarAFDs(const AFD *a1, const AFD *a2, AFD *saida) {
  AFD r = {0};
  int pares1[MAX_ESTADOS], pares2[MAX_ESTADOS];
  if (!saida || !valido(a1) || !valido(a2)) return 0;
  /* Uniao dos alfabetos; associa simbolos pelo valor, nao pela coluna. */
  for (int i = 0; i < a1->m; ++i) r.alfabeto[r.m++] = a1->alfabeto[i];
  for (int i = 0; i < a2->m; ++i) {
    int s = 0;
    while (s < r.m && r.alfabeto[s] != a2->alfabeto[i]) ++s;
    if (s == r.m) {
      if (r.m == MAX_SIMBOLOS) return 0;
      r.alfabeto[r.m++] = a2->alfabeto[i];
    }
  }
  r.n = 1;
  r.inicial = 0;
  pares1[0] = a1->inicial;
  pares2[0] = a2->inicial;
  /* Busca em largura: o vetor de pares tambem funciona como fila. */
  for (int u = 0; u < r.n; ++u) {
    int p = pares1[u], q = pares2[u];
    r.final[u] = p >= 0 && q >= 0 && a1->final[p] && a2->final[q];
    for (int s = 0; s < r.m; ++s) {
      int np = passo(a1, p, r.alfabeto[s]);
      int nq = passo(a2, q, r.alfabeto[s]);
      int v = 0;
      while (v < r.n && (pares1[v] != np || pares2[v] != nq)) ++v;
      if (v == r.n) {
        if (r.n == MAX_ESTADOS) return 0;
        pares1[v] = np;
        pares2[v] = nq;
        ++r.n;
      }
      r.delta[u][s] = v;
    }
  }
  *saida = r;
  return 1;
}

static int validoAFN(const AFN *a) {
  if (!a || a->n < 1 || a->n > MAX_AFN_ESTADOS || a->m < 1 ||
      a->m > MAX_SIMBOLOS || a->inicial < 0 || a->inicial >= a->n) return 0;
  uint64_t permitidos = a->n == 64 ? UINT64_MAX : (UINT64_C(1) << a->n) - 1;
  uint64_t finais = 0;
  for (int p = 0; p < a->n; ++p) {
    if (a->final[p] != 0 && a->final[p] != 1) return 0;
    if (a->final[p]) finais |= UINT64_C(1) << p;
    for (int s = 0; s < a->m; ++s)
      if (a->trans[p][s] & ~permitidos) return 0;
  }
  if (finais != a->finais_mask) return 0;
  for (int s = 0; s < a->m; ++s) {
    if (!isgraph((unsigned char)a->alfabeto[s])) return 0;
    for (int t = 0; t < s; ++t)
      if (a->alfabeto[s] == a->alfabeto[t]) return 0;
  }
  return 1;
}

int intersectarAFNs(const AFN *a1, const AFN *a2, AFD *saida) {
  AFD d1, d2;
  if (!saida || !validoAFN(a1) || !validoAFN(a2)) return 0;
  return determinizarAFNCompleto(a1, &d1) &&
         determinizarAFNCompleto(a2, &d2) && intersectarAFDs(&d1, &d2, saida);
}

static void descartarLinha(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {}
}

/* 1 = linha inteira; 0 = EOF; -1 = linha longa descartada. */
static int lerLinha(FILE *f, char *buf, int tamanho) {
  if (!fgets(buf, tamanho, f)) return 0;
  if (!strchr(buf, '\n') && !feof(f)) {
    int c;
    while ((c = fgetc(f)) != '\n' && c != EOF) {}
    return -1;
  }
  buf[strcspn(buf, "\r\n")] = 0;
  return 1;
}

static int inteiro(const char *s, int maximo, int *valor) {
  char *fim;
  if (!s || !*s) return 0;
  errno = 0;
  long n = strtol(s, &fim, 10);
  if (errno || *fim || n < 0 || n > maximo) return 0;
  *valor = (int)n;
  return 1;
}

/* Formato do grupo, com validacao estrita para nao ignorar transicoes ruins.
   Aceita comentarios em linha propria e varios destinos por linha. */
static int lerArquivo(const char *nome, AFN *a) {
  FILE *f = fopen(nome, "r");
  if (!f) { printf("Nao foi possivel abrir '%s'.\n", nome); return 0; }
  AFN r = {0};
  char linha[2048], *tokens[128];
  int etapa = 0, status, ok = 1;
  while ((status = lerLinha(f, linha, sizeof linha)) != 0) {
    if (status < 0) { ok = 0; break; }
    char *p = linha;
    while (isspace((unsigned char)*p)) ++p;
    if (!*p || *p == '#' || (p[0] == '/' && p[1] == '/')) continue;
    int nt = 0;
    for (char *t = strtok(p, " \t,"); t; t = strtok(NULL, " \t,")) {
      if (nt == 128) { ok = 0; break; }
      tokens[nt++] = t;
    }
    if (!ok || nt == 0) { ok = 0; break; }
    if (etapa == 0) {
      ok = nt == 2 && inteiro(tokens[0], MAX_AFN_ESTADOS, &r.n) && r.n > 0 &&
           inteiro(tokens[1], MAX_SIMBOLOS, &r.m) && r.m > 0;
    } else if (etapa == 1) {
      ok = nt == r.m;
      for (int s = 0; ok && s < nt; ++s) {
        ok = strlen(tokens[s]) == 1;
        r.alfabeto[s] = tokens[s][0];
      }
    } else if (etapa == 2) {
      ok = nt == 1 && inteiro(tokens[0], r.n - 1, &r.inicial);
    } else if (etapa == 3) {
      int nf;
      ok = inteiro(tokens[0], r.n, &nf) && nt == nf + 1;
      for (int i = 1; ok && i < nt; ++i) {
        int q;
        ok = inteiro(tokens[i], r.n - 1, &q);
        if (ok) { r.final[q] = 1; r.finais_mask |= UINT64_C(1) << q; }
      }
    } else {
      int origem, s = 0;
      ok = nt >= 3 && inteiro(tokens[0], r.n - 1, &origem) && strlen(tokens[1]) == 1;
      if (ok) {
        while (s < r.m && r.alfabeto[s] != tokens[1][0]) ++s;
        ok = s < r.m;
      }
      for (int i = 2; ok && i < nt; ++i) {
        int destino;
        if (!strcmp(tokens[i], "-") && nt == 3) continue;
        ok = inteiro(tokens[i], r.n - 1, &destino);
        if (ok) r.trans[origem][s] |= UINT64_C(1) << destino;
      }
    }
    if (!ok) break;
    ++etapa;
  }
  if (ferror(f)) ok = 0;
  fclose(f);
  if (!ok || etapa < 4 || !validoAFN(&r)) {
    puts("Arquivo invalido: confira cabecalho, finais e transicoes (sem lambda).");
    return 0;
  }
  *a = r;
  return 1;
}

static int lerEntrada(int numero, AFN *a) {
  int opcao;
  char nome[1024];
  for (;;) {
    printf("\nAF %d: 1 - Teclado | 2 - Arquivo (.txt / .json) | 0 - Cancelar\nOpcao: ", numero);
    fflush(stdout);
    int leitura = lerLinha(stdin, nome, sizeof nome);
    if (!leitura) return 0;
    if (leitura < 0 || !inteiro(nome, 2, &opcao)) {
      puts("Escolha 1, 2 ou 0. A leitura continua aqui.");
      continue;
    }
    if (opcao == 0) return 0;
    if (opcao == 1) {
      int ok = lerAFN(a);
      descartarLinha();
      if (ok && validoAFN(a)) return 1;
      if (feof(stdin)) return 0;
      puts("Dados invalidos. Escolha novamente como informar este automato.");
      continue;
    }
    printf("Exemplo: exemplo_interseccao_afn_m%d.txt ou exemplo_leitura.json\n", numero);
    for (;;) {
      printf("Arquivo de M%d (0 = cancelar): ", numero);
      fflush(stdout);
      leitura = lerLinha(stdin, nome, sizeof nome);
      if (!leitura) return 0;
      if (leitura < 0) { puts("Caminho muito longo. Tente novamente."); continue; }
      char *caminho = nome;
      while (isspace((unsigned char)*caminho)) ++caminho;
      size_t n = strlen(caminho);
      while (n && isspace((unsigned char)caminho[n - 1])) caminho[--n] = 0;
      if (n >= 2 && ((caminho[0] == '"' && caminho[n - 1] == '"') ||
                     (caminho[0] == '\'' && caminho[n - 1] == '\''))) {
        caminho[n - 1] = 0;
        ++caminho;
      }
      if (!strcmp(caminho, "0")) return 0;
      if (!*caminho) { puts("Nenhum arquivo informado. Digite o caminho e pressione ENTER."); continue; }
      if (strstr(caminho, ".json") != NULL) {
        if (lerAFNJson(caminho, a, numero) && validoAFN(a)) {
          printf("M%d carregado com sucesso do JSON: %d estados e %d simbolos.\n", numero, a->n, a->m);
          return 1;
        }
      } else if (lerArquivo(caminho, a)) {
        printf("M%d carregado com sucesso: %d estados e %d simbolos.\n", numero, a->n, a->m);
        return 1;
      }
      puts("Nao foi possivel carregar este AF. Confira o caminho/formato e tente novamente.");
    }
  }
}

void menuInterseccaoAutomatos(void) {
  AFN a1, a2, equivalente = {0};
  AFD d1, d2, resultado, minimo;
  /* O menu principal usa scanf: consumir apenas o restante dessa linha uma vez. */
  descartarLinha();
  puts("\n=== INTERSECCAO DE DOIS AFs (AFN/AFD, SEM LAMBDA) ===");
  puts("Vamos encontrar as palavras que os dois automatos aceitam em comum.");

  puts("\nComo deseja informar os automatos para a interseccao?");
  puts("1 - Ler ambos de um unico arquivo .json (ex: exemplo_leitura.json)");
  puts("2 - Informar AF 1 e AF 2 separadamente (teclado / .txt / .json)");
  printf("Opcao [padrao: 1]: ");

  char linha_opcao[256];
  int modo = 1;
  if (lerLinha(stdin, linha_opcao, sizeof linha_opcao) && linha_opcao[0]) {
    inteiro(linha_opcao, 2, &modo);
  }
  if (modo != 1 && modo != 2) modo = 1;

  int carregado = 0;
  if (modo == 1) {
    char caminho_json[1024];
    printf("Nome do arquivo .json (ex: exemplo_leitura.json): ");
    fflush(stdout);
    if (lerLinha(stdin, caminho_json, sizeof caminho_json) && caminho_json[0]) {
      char *p = caminho_json;
      while (isspace((unsigned char)*p)) ++p;
      size_t n = strlen(p);
      while (n && isspace((unsigned char)p[n - 1])) p[--n] = 0;
      if (n >= 2 && ((p[0] == '"' && p[n - 1] == '"') || (p[0] == '\'' && p[n - 1] == '\''))) {
        p[n - 1] = 0;
        ++p;
      }
      if (lerDoisAFNsJson(p, &a1, &a2) && validoAFN(&a1) && validoAFN(&a2)) {
        printf("M1 carregado (%d estados, %d simbolos) e M2 carregado (%d estados, %d simbolos).\n",
               a1.n, a1.m, a2.n, a2.m);
        carregado = 1;
      } else {
        puts("Falha ao carregar os 2 automatos do JSON informado.");
      }
    }
  } else {
    carregado = lerEntrada(1, &a1) && lerEntrada(2, &a2);
  }

  if (!carregado) {
    puts("Operacao cancelada ou entrada invalida. Retornando ao menu.");
    return;
  }
  if (!determinizarAFNCompleto(&a1, &d1) || !determinizarAFNCompleto(&a2, &d2) ||
      !intersectarAFDs(&d1, &d2, &resultado)) {
    printf("Interseccao excede os limites: %d estados / %d simbolos.\n",
           MAX_ESTADOS, MAX_SIMBOLOS);
    return;
  }
  puts("\n--- INTERSECCAO: L(M1) & L(M2) ---"); imprimirAFD(&resultado);
  int finais = 0;
  for (int i = 0; i < resultado.n; ++i) finais += !!resultado.final[i];
  if (!finais) puts("Linguagem vazia: nenhum estado final alcancavel.");
  int tem_afn = converterAFDparaAFN(&resultado, &equivalente);
  int tem_minimo = minimizarAFD(&resultado, &minimo);
  for (;;) {
    int op;
    char linha[1024];
    puts("\nOpcoes adicionais:");
    puts("1 - Testar palavras em M1, M2 e na Interseccao");
    puts("2 - Salvar AFD Interseccao em arquivo .txt");
    puts("3 - Salvar AFN Interseccao equivalente em arquivo .txt");
    puts("4 - Salvar AFD Interseccao minimizado em arquivo .txt");
    puts("5 - Ver etapas da construcao");
    puts("0 - Voltar ao menu principal");
    printf("Escolha: ");
    int leitura = lerLinha(stdin, linha, sizeof linha);
    if (!leitura) return;
    if (leitura < 0 || !inteiro(linha, 5, &op)) { puts("Opcao invalida."); continue; }
    if (!op) return;
    if (op == 5) {
      puts("\n--- 1) AF ORIGINAL M1 ---"); imprimirAFN(&a1);
      puts("\n--- 2) AF ORIGINAL M2 ---"); imprimirAFN(&a2);
      puts("\n--- 3) AFD M1 DETERMINIZADO E COMPLETO ---"); imprimirAFD(&d1);
      puts("\n--- 4) AFD M2 DETERMINIZADO E COMPLETO ---"); imprimirAFD(&d2);
      if (tem_afn) {
        puts("\n--- AFN INTERSECCAO EQUIVALENTE ---"); imprimirAFN(&equivalente);
      } else puts("AFN equivalente indisponivel: resultado excede 64 estados.");
      if (tem_minimo) {
        puts("\n--- AFD INTERSECCAO MINIMIZADO (MOORE) ---"); imprimirAFD(&minimo);
      }
      continue;
    }
    if (op == 1) {
      puts("Digite uma palavra (ENTER ou '_' = vazia; 'fim' = voltar).");
      puts("Use '=' antes de uma palavra literal reservada, por exemplo '=fim' ou '=_'.");
      for (;;) {
        printf("Palavra: ");
        leitura = lerLinha(stdin, linha, sizeof linha);
        if (!leitura) return;
        if (leitura < 0) { puts("Palavra muito longa (maximo 1022 caracteres)."); continue; }
        if (!strcmp(linha, "fim")) break;
        char *w = linha;
        if (*w == '=') ++w;
        else if (!strcmp(w, "_") || !strcmp(w, "lambda") || !strcmp(w, "vazio")) *w = 0;
        int r1 = simularPalavraAFD(&d1, w), r2 = simularPalavraAFD(&d2, w);
        int ri = simularPalavraAFD(&resultado, w);
        printf("-> Palavra \"%s\":\n", *w ? w : "<vazia>");
        printf("   M1: %s\n", r1 == 1 ? "ACEITA" : "REJEITADA");
        printf("   M2: %s\n", r2 == 1 ? "ACEITA" : "REJEITADA");
        printf("   Interseccao: %s\n", ri < 0 ? "SIMBOLO FORA DO ALFABETO" : ri ? "ACEITA" : "REJEITADA");
        if (ri >= 0) printf("   Consistencia: %s\n", ri == (r1 == 1 && r2 == 1) ? "CORRETO (M1 E M2)" : "FALHA");
        /* Simbolo ausente em um operando representa rejeicao nesse operando. */
      }
    } else {
      if (op == 3 && !tem_afn) { puts("AFN indisponivel: limite de 64 estados."); continue; }
      if (op == 4 && !tem_minimo) { puts("AFD minimizado indisponivel."); continue; }
      const AFD *salvar = op == 4 ? &minimo : &resultado;
      if (op != 3 && salvar->n >= MAX_ESTADOS) {
        puts("Leitor AFD do projeto aceita ate 99 estados. Tente salvar o minimizado (4).");
        continue;
      }
      printf("Caminho para salvar (sem aspas): ");
      if (lerLinha(stdin, linha, sizeof linha) != 1) { if (feof(stdin)) return; continue; }
      FILE *existente = fopen(linha, "r");
      if (existente) {
        fclose(existente);
        puts("Arquivo ja existe. Escolha outro nome para preservar seu conteudo.");
        continue;
      }
      int ok = op == 3 ? salvarAFNEmArquivo(linha, &equivalente) : salvarAFDEmArquivo(linha, salvar);
      if (ok) printf("Resultado salvo em '%s'.\n", linha);
    }
  }
}
