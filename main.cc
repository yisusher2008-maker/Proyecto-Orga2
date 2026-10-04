#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "cpu.h"
#include "logger.h"

/*Función auxiliar: crea un archivo .asm a partir de la entrada
por teclado. El usuario escribe líneas y termina con "FIN".*/

static void crear_archivo_desde_teclado(const char *nombre_archivo) {
    FILE *f = fopen(nombre_archivo, "w");
    if (!f) {
        printf("Error: No se pudo crear el archivo temporal %s\n", nombre_archivo);
        return;
    }

    printf("\n--- Escriba su codigo ensamblador ---\n");
    printf("--- Escriba 'FIN' en una linea sola para terminar ---\n\n");

    char linea[256];
    while (1) {
        printf("> ");
        if (!fgets(linea, sizeof(linea), stdin)) break;
        if (strncmp(linea, "FIN", 3) == 0) break;
        fputs(linea, f);
    }

    fclose(f);
    printf("\nArchivo temporal '%s' creado correctamente.\n", nombre_archivo);
}

//función principal del programa:
int main(int argc, char *argv[]) {
    char archivo_asm[256] = {0};
    char archivo_txt[300] = {0};
    const char *archivo_log = "trace.log";

    if (argc > 1) {
        // Entrada por parámetro de línea de comandos
        strncpy(archivo_asm, argv[1], sizeof(archivo_asm) - 1);
        archivo_asm[sizeof(archivo_asm) - 1] = '\0';

        if (strstr(archivo_asm, ".asm") == NULL) {
            printf("Error: El archivo '%s' debe tener extension .asm\n", archivo_asm);
            return 1;
        }
    } else {
        // Menú interactivo
        int opcion = 0;

        printf("  Emulador Proyecto: \n");
        printf("Seleccione modo de entrada:\n");
        printf("  1. Cargar archivo .asm existente\n");
        printf("  2. Escribir codigo ensamblador por teclado\n");
        printf("Opcion: ");

        if (scanf("%d", &opcion) != 1) {
            opcion = 1;
        }
        // Limpiar el buffer después del scanf
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        if (opcion == 2) {
            // Entrada por teclado
            strcpy(archivo_asm, "codigo_teclado.asm");
            crear_archivo_desde_teclado(archivo_asm);
        } else {
            // Entrada por archivo
            printf("Ingrese el nombre del archivo (.asm): ");
            if (scanf("%255s", archivo_asm) != 1) {
                printf("Error al leer el nombre del archivo.\n");
                return 1;
            }
            // Limpiar el buffer
            while ((c = getchar()) != '\n' && c != EOF);

            // Validar extensión .asm
            if (strstr(archivo_asm, ".asm") == NULL) {
                printf("Error: El archivo debe tener extension .asm\n");
                return 1;
            }
        }
    }

//función auxiliar para generar el nombre del archivo de salida .txt a partir del archivo .asm:
    snprintf(archivo_txt, sizeof(archivo_txt), "%s.txt", archivo_asm);

//fase ensamblador:

    printf(" Fase 1: Ensamblador\n");

    uint32_t dir_inicio = calcular_direccion_inicio();

    primera_pasada(archivo_asm, dir_inicio);
    segunda_pasada_y_generar_txt(archivo_asm, archivo_txt, dir_inicio);

    printf("\nArchivo ensamblado generado: %s\n", archivo_txt);

//fase emulador de CPU:
    printf(" Fase 2: Emulador de CPU\n");

    cpu procesador;
    inicializar_cpu(&procesador, dir_inicio);
    cargar_memoria(&procesador, archivo_txt, dir_inicio);

    // Abrir el archivo de log
    FILE *log_file = iniciar_log(archivo_log);
    if (!log_file) {
        printf("Error: No se pudo abrir el archivo de log '%s'\n", archivo_log);
        return 1;
    }

    // Ejecutar el ciclo de la CPU
    ciclo_cpu(&procesador, log_file);

    // Cerrar el log
    cerrar_log(log_file);

//mostrar estado final de la CPU y mensaje de finalización:
    printf(" Simulación finalizada.\n");
    printf("Estado final de STAT: 0x%X\n", procesador.stat);

    switch (procesador.stat) {
        case stat_aok:
            printf("  (AOK: ejecucion normal)\n");
            break;
        case stat_hlt:
            printf("  (HLT: apagado normal via HALT)\n");
            break;
        case stat_ins:
            printf("  (INS: instruccion ilegal)\n");
            break;
        case stat_aex:
            printf("  (AEX: error de alineacion)\n");
            break;
        case stat_adr:
            printf("  (ADR: direccion invalida)\n");
            break;
        default:
            printf("  (Estado desconocido)\n");
            break;
    }
    printf("\nArchivo de traza: %s\n", archivo_log);
    printf("Archivo ensamblado: %s\n", archivo_txt);
    return 0;
}