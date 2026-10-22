#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "commons.h"
#include "conversor_AFN_para_AFD.h"

int main() {
    int op = -1;
    AFN afn;
    AFD dfa;
    bool automato_carregado = false;
    char caminho_arquivo[256];
    char buffer_entrada[256];

    inicializar_afn(&afn);
    inicializar_afd(&dfa);

    while (op != 0) {
        limparTela();
        imprimirCabecalho("SIMULADOR DE AUTOMATOS - LFA");
        printf("  1 - Converter AFN para AFD (Ler de arquivo .txt)\n");
        printf("  2 - Testar palavra no AFD gerado\n");
        printf("  3 - Exibir tabelas do AFN e AFD atuais\n");
        printf("  4 - Salvar AFD gerado em arquivo .txt\n");
        printf("  0 - Sair\n");
        printf("------------------------------------------------------------\n");
        if (automato_carregado) {
            printf("  [Status: Automato carregado (%d est. AFN -> %d est. AFD)]\n", 
                   afn.num_estados_nfa, dfa.total_dfa_estados);
        } else {
            printf("  [Status: Nenhum automato carregado na memoria]\n");
        }
        printf("------------------------------------------------------------\n");
        printf("Digite a opcao desejada: ");

        if (fgets(buffer_entrada, sizeof(buffer_entrada), stdin) == NULL) {
            break;
        }

        if (sscanf(buffer_entrada, "%d", &op) != 1) {
            printf("\nOpcao invalida! Digite um numero.\n");
            pausarTela();
            continue;
        }

        switch (op) {
            case 1: {
                limparTela();
                imprimirCabecalho("CONVERSOR AFN -> AFD (VIA TXT)");
                printf("Arquivos de exemplo disponiveis:\n");
                printf(" - exemplo_afn_ab.txt (Reconhece palavras terminadas em 'ab')\n");
                printf(" - exemplo_afn_01.txt (Reconhece palavras contendo '01')\n\n");
                printf("Digite o nome ou caminho do arquivo .txt [Padrao: exemplo_afn_ab.txt]: ");

                if (fgets(caminho_arquivo, sizeof(caminho_arquivo), stdin) != NULL) {
                    caminho_arquivo[strcspn(caminho_arquivo, "\r\n")] = '\0';
                }

                // Se o usuário apenas apertou Enter, usa o arquivo padrão
                if (strlen(caminho_arquivo) == 0) {
                    strcpy(caminho_arquivo, "exemplo_afn_ab.txt");
                }

                if (carregar_afn_arquivo(caminho_arquivo, &afn)) {
                    converter_afn_para_afd(&afn, &dfa);
                    automato_carregado = true;

                    // Exibe o AFN original
                    imprimir_afn(&afn);

                    // Exibe o AFD resultante
                    imprimir_afd(&afn, &dfa);

                    printf("\nConversao concluida com sucesso!\n");
                } else {
                    printf("\nFalha ao carregar o arquivo '%s'. Verifique se o caminho esta correto.\n", caminho_arquivo);
                }

                pausarTela();
                break;
            }

            case 2: {
                limparTela();
                imprimirCabecalho("SIMULADOR DE PALAVRA NO AFD");
                if (!automato_carregado) {
                    printf("\n[AVISO] Nenhum automato foi convertido ainda!\n");
                    printf("Por favor, execute a Opcao 1 primeiro para carregar e converter um AFN.\n");
                    pausarTela();
                    break;
                }

                while (true) {
                    printf("\nDigite a cadeia para testar no AFD (ou digite 'sair' para voltar): ");
                    if (fgets(buffer_entrada, sizeof(buffer_entrada), stdin) == NULL) {
                        break;
                    }
                    buffer_entrada[strcspn(buffer_entrada, "\r\n")] = '\0';

                    if (strcmp(buffer_entrada, "sair") == 0) {
                        break;
                    }

                    simular_cadeia_afd(&afn, &dfa, buffer_entrada);
                }
                break;
            }

            case 3: {
                limparTela();
                if (!automato_carregado) {
                    imprimirCabecalho("VISUALIZADOR DE AUTOMATOS");
                    printf("\n[AVISO] Nenhum automato carregado ainda!\n");
                    printf("Por favor, carregue um automato na Opcao 1 primeiro.\n");
                } else {
                    imprimir_afn(&afn);
                    imprimir_afd(&afn, &dfa);
                }
                pausarTela();
                break;
            }

            case 4: {
                limparTela();
                imprimirCabecalho("EXPORTAR AFD PARA ARQUIVO TXT");
                if (!automato_carregado) {
                    printf("\n[AVISO] Nenhum automato carregado para exportar!\n");
                    printf("Por favor, carregue um automato na Opcao 1 primeiro.\n");
                    pausarTela();
                    break;
                }

                char caminho_saida[256];
                printf("Digite o nome do arquivo para salvar [Padrao: afd_convertido.txt]: ");
                if (fgets(caminho_saida, sizeof(caminho_saida), stdin) != NULL) {
                    caminho_saida[strcspn(caminho_saida, "\r\n")] = '\0';
                }
                if (strlen(caminho_saida) == 0) {
                    strcpy(caminho_saida, "afd_convertido.txt");
                }

                salvar_afd_arquivo(caminho_saida, &afn, &dfa);
                pausarTela();
                break;
            }

            case 0:
                limparTela();
                printf("\nEncerrando o Simulador de Automatos. Ate mais!\n\n");
                break;

            default:
                printf("\nOpcao invalida!\n");
                pausarTela();
                break;
        }
    }

    return 0;
}