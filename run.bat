@echo off
echo Compilando o Simulador de Automatos...
gcc -Wall -Wextra *.c -o exe.exe
if %ERRORLEVEL% equ 0 (
    echo Iniciando o simulador...
    exe.exe
) else (
    echo [ERRO] Falha na compilacao.
    pause
)
