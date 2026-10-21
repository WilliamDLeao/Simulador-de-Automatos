#include <stdio.h>
#include <stdlib.h>
#include "commons.h"

void limparTela() {
    #ifdef _WIN32
        system("cls");   // Windows
    #else
        system("clear"); 
    #endif
}

void imprimirCabecalho(const char *titulo) {
    printf("\n============================================================\n");
    printf("                  %s\n", titulo);
    printf("============================================================\n");
}

void pausarTela() {
    printf("\nPressione [ENTER] para continuar...");
    char temp[64];
    if (fgets(temp, sizeof(temp), stdin) == NULL) {
        // EOF
    }
}