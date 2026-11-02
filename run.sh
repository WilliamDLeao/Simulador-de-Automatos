#!/bin/bash
set -e

# Compila o projeto (apenas arquivos alterados via make)
make --no-print-directory

# Executa o simulador
./exe
