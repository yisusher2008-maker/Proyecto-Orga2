#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "cpu.h"
#include "logger.h"

// Función auxiliar si el usuario prefiere escribir el código directamente en pantalla
void crear_archivo_desde_teclado(const char *nombre_archivo) {
    FILE *f = fopen(nombre_archivo, "w");
    if (!f) {
        printf("Error al crear archivo temporal.\n");
        return;
    }
    printf("\n--- Escriba su codigo ensamblador (Escriba 'FIN' en una linea sola para terminar) ---\n");
    char linea[256];
    getchar(); // Limpia el buffer
    while (1) {
        printf("> ");
        if (!fgets(linea, sizeof(linea), stdin)) break;
        if (strncmp(linea, "FIN", 3) == 0) break;
        fputs(linea, f);
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    char archivo_asm[256] = "codigo_teclado.asm";
    const char *archivo_txt = "programa_ensamblado.txt";
    const char *archivo_log = "trace.log";

    if (argc > 1) {
        // Entrada por parámetro de consola
        strncpy(archivo_asm, argv[1], sizeof(archivo_asm) - 1);
    } else {
        // Menú para seleccionar entre Archivo o Teclado
        int opcion = 0;
        printf("=== SELECCIONE MODO DE ENTRADA ===\n");
        printf("1. Cargar archivo .asm existente\n");
        printf("2. Escribir codigo ensamblador por teclado\n");
        printf("Opcion: ");
        if (scanf("%d", &opcion) != 1) opcion = 1;

        if (opcion == 2) {
            crear_archivo_desde_teclado(archivo_asm);
        } else {
            printf("Ingrese el nombre del archivo (.asm): ");
            scanf("%255s", archivo_asm);
        }
    }

    printf("\n========================================\n");
    printf("     FASE 1: MODULO ENSAMBLADOR\n");
    printf("========================================\n\n");

    uint32_t dir_inicio = calcular_direccion_inicio();

    primera_pasada(archivo_asm, dir_inicio);
    segunda_pasada_y_generar_txt(archivo_asm, archivo_txt, dir_inicio);

    printf("\n========================================\n");
    printf("     FASE 2: EMULADOR DE CPU\n");
    printf("========================================\n\n");

    cpu procesador;
    inicializar_cpu(&procesador, dir_inicio);
    cargar_memoria(&procesador, archivo_txt, dir_inicio);

    FILE *log_file = abrir_logger(archivo_log);
    if (!log_file) return 1;

    ciclo_cpu(&procesador, log_file);
    cerrar_logger(log_file);

    printf("\nSimulacion finalizada. Traza guardada en %s\n", archivo_log);
    return 0;
}
