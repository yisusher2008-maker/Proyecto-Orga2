#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include "assembler.h"

//variables globales del módulo ensamblador:
Simbolo tabla_simbolos[max_simbolos];
int total_simbolos = 0;

//cálculo de la dirección de inicio:
uint32_t calcular_direccion_inicio(void) {
    uint32_t c1 = cedula_1;
    uint32_t c2 = cedula_2;
    uint32_t c3 = cedula_3;

    uint64_t suma = (uint64_t)c1 + (uint64_t)c2 + (uint64_t)c3;
    uint32_t direccion_inicio = (uint32_t)(suma & 0xFFFFFFFF);

    printf("Calculo de la direccion de inicio:\n");
    printf("Cedula Integrante 1: %u\n", c1);
    printf("Cedula Integrante 2: %u\n", c2);
    printf("Cedula Integrante 3: %u\n", c3);
    printf("----------------------------------------\n");
    printf("Suma total:          %llu\n", (unsigned long long)suma);
    printf("Suma en hexadecimal: 0x%llX\n", (unsigned long long)suma);
    printf("Mascara 0xFFFFFFFF:  0x%08X\n", 0xFFFFFFFF);
    printf("Direccion de inicio: 0x%08X  (32 bits menos significativos)\n", direccion_inicio);
    printf("----------------------------------------\n");
    return direccion_inicio;
}

//función auxiliar para buscar una etiqueta en la tabla:
uint32_t buscar_direccion_etiqueta(const char* nombre_etiqueta) {
    for (int i = 0; i < total_simbolos; i++) {
        if (strcmp(tabla_simbolos[i].etiqueta, nombre_etiqueta) == 0) {
            return tabla_simbolos[i].direccion;
        }
    }
    return 0xFFFFFFFF;
}
//función auxiliar para limpiar comentarios y saltos de línea:
static int limpiar_linea(char *linea) {
    char *comentario1 = strchr(linea, '#');
    char *comentario2 = strchr(linea, ';');
    if (comentario1) *comentario1 = '\0';
    if (comentario2) *comentario2 = '\0';
    linea[strcspn(linea, "\r\n")] = 0;

    for (int i = 0; linea[i] != '\0'; i++) {
        if (!isspace((unsigned char)linea[i])) return 0;
    }
    return 1;
}

//función para generar la primera pasada y llenar la tabla de símbolos:
void primera_pasada(const char* archivo_asm, uint32_t direccion_inicio) {
    FILE *archivo = fopen(archivo_asm, "r");
    if (archivo == NULL) {
        printf("Error critico: No se pudo abrir el archivo %s\n", archivo_asm);
        return;
    }

    char linea[256];
    uint32_t pc_actual = direccion_inicio;
    total_simbolos = 0;

    while (fgets(linea, sizeof(linea), archivo)) {
        if (limpiar_linea(linea)) continue;

        char *dos_puntos = strchr(linea, ':');
        int tiene_instruccion = 0;

        if (dos_puntos != NULL) {
            int longitud = dos_puntos - linea;

            if (longitud >= max_etiqueta) {
                printf("Error: Etiqueta demasiado larga: %s\n", linea);
                fclose(archivo);
                return;
            }

            char etiqueta_temp[max_etiqueta];
            strncpy(etiqueta_temp, linea, longitud);
            etiqueta_temp[longitud] = '\0';

            int fin = longitud - 1;
            while (fin >= 0 && isspace((unsigned char)etiqueta_temp[fin])) {
                etiqueta_temp[fin] = '\0';
                fin--;
            }

            for (int i = 0; i < total_simbolos; i++) {
                if (strcmp(tabla_simbolos[i].etiqueta, etiqueta_temp) == 0) {
                    printf("Error: Etiqueta duplicada '%s'\n", etiqueta_temp);
                    fclose(archivo);
                    return;
                }
            }

            if (total_simbolos >= max_simbolos) {
                printf("Error: Se excedio el maximo de simbolos (%d)\n", max_simbolos);
                fclose(archivo);
                return;
            }

            strcpy(tabla_simbolos[total_simbolos].etiqueta, etiqueta_temp);
            tabla_simbolos[total_simbolos].direccion=pc_actual;
            total_simbolos++;

            char *resto=dos_puntos+1;
            for (int i = 0; resto[i]!='\0'; i++) {
                if (!isspace((unsigned char)resto[i])) {
                    tiene_instruccion=1;
                    break;
                }
            }
        } else {
            tiene_instruccion=1;
        }

        if(tiene_instruccion) {
            pc_actual+=8;
        }
    }
    fclose(archivo);
    printf("Primera pasada completada. Simbolos encontrados: %d\n", total_simbolos);
}

//función para extraer el número de registro:
static uint8_t parsear_registro(const char* reg_str) {
    for (int i = 0; reg_str[i] != '\0'; i++) {
        if (isdigit((unsigned char)reg_str[i])) {
            return (uint8_t)(reg_str[i] - '0');
        }
    }
    return 0xF;
}

//función para parsear memoria en desplazamiento:
static int parsear_memoria(const char* op, int32_t *desplazamiento, uint8_t *rB) {
    const char *parentesis = strchr(op, '(');
    if (parentesis == NULL) return 0;

    char desp_str[32] = {0};
    int len = parentesis - op;
    if (len >= 32) len = 31;
    strncpy(desp_str, op, len);
    desp_str[len] = '\0';

    char *p = desp_str;
    while (*p == ' ' || *p == '$') p++;
    *desplazamiento = atoi(p);

    const char *reg = parentesis + 1;
    *rB = parsear_registro(reg);
    return 1;
}

//función para la segunda pasada y generar el archivo .txt:
void segunda_pasada_y_generar_txt(const char* archivo_asm, const char* archivo_txt, uint32_t direccion_inicio) {
    FILE *entrada = fopen(archivo_asm, "r");
    FILE *salida = fopen(archivo_txt, "w");

    if (!entrada || !salida) {
        printf("Error critico: No se pudieron abrir los archivos.\n");
        if (entrada) fclose(entrada);
        if (salida) fclose(salida);
        return;
    }

    uint32_t c1 = cedula_1, c2 = cedula_2, c3 = cedula_3;
    uint64_t suma = (uint64_t)c1 + (uint64_t)c2 + (uint64_t)c3;
    uint32_t dir_calc = (uint32_t)(suma & 0xFFFFFFFF);

    fprintf(salida, "Calculo de la direccion de inicio\n");
    fprintf(salida, "Cedula Integrante 1: %u\n", c1);
    fprintf(salida, "Cedula Integrante 2: %u\n", c2);
    fprintf(salida, "Cedula Integrante 3: %u\n", c3);
    fprintf(salida, "----------------------------------------\n");
    fprintf(salida, "Suma total:          %llu\n", (unsigned long long)suma);
    fprintf(salida, "Suma en hexadecimal: 0x%llX\n", (unsigned long long)suma);
    fprintf(salida, "Mascara 0xFFFFFFFF:  0x%08X\n", 0xFFFFFFFF);
    fprintf(salida, "Direccion de inicio: 0x%08X  (32 bits menos significativos)\n", dir_calc);
    fprintf(salida, "----------------------------------------\n");

//tabla de símbolos:
    fprintf(salida, "--- TABLA DE SIMBOLOS ---\n");
    for (int i = 0; i < total_simbolos; i++) {
        fprintf(salida, "%s: 0x%08X\n", tabla_simbolos[i].etiqueta, tabla_simbolos[i].direccion);
    }
    fprintf(salida, "-------------------------\n\n");

//encabezado de la sección de instrucciones:
    fprintf(salida, "Direccion  | Codigo de maquina (Hex) | # Instruccion\n");
    fprintf(salida, "------------------------------------------------------\n");

    char linea[256];
    char linea_original[256];
    uint32_t pc_actual = direccion_inicio;

    while (fgets(linea, sizeof(linea), entrada)) {
        strcpy(linea_original, linea);
        linea_original[strcspn(linea_original, "\r\n")] = 0;

        if (limpiar_linea(linea)) continue;

        char *instruccion_str = linea;
        char *dos_puntos = strchr(linea, ':');
        if (dos_puntos != NULL) {
            instruccion_str = dos_puntos + 1;
        }

        int vacio = 1;
        for (int i = 0; instruccion_str[i] != '\0'; i++) {
            if (!isspace((unsigned char)instruccion_str[i])) { vacio = 0; break; }
        }
        if (vacio) continue;

        uint8_t icode = 0xF, ifun = 0x0, rA = 0xF, rB = 0xF;
        uint64_t valC = 0;

        char mnemonico[20] = {0}, op1[50] = {0}, op2[50] = {0};
        sscanf(instruccion_str, "%s %[^,], %s", mnemonico, op1, op2);

//decodificación de instrucciones:
        if (strcmp(mnemonico, "HALT") == 0) {
            icode = 0xE; ifun = 0x0;
        }
        else if (strcmp(mnemonico, "NOP") == 0) {
            icode = 0xF; ifun = 0x0;
        }
        else if (strcmp(mnemonico, "RRMVQ") == 0) {
            icode = 0x5; ifun = 0x0;
            rA = parsear_registro(op1);
            rB = parsear_registro(op2);
        }
        else if (strcmp(mnemonico, "IRMOVQ") == 0) {
            icode = 0x1; ifun = 0x0;
            rB = parsear_registro(op2);
            char *p = op1;
            while (*p == ' ' || *p == '$') p++;
            valC = (uint64_t)strtoull(p, NULL, 0);
        }
        else if (strcmp(mnemonico, "ADDQ") == 0) {
            icode = 0xA; ifun = 0x2;
            rA = parsear_registro(op1); rB = parsear_registro(op2);
        }
        else if (strcmp(mnemonico, "SUBQ") == 0) {
            icode = 0xA; ifun = 0x3;
            rA = parsear_registro(op1); rB = parsear_registro(op2);
        }
        else if (strcmp(mnemonico, "ANDQ") == 0) {
            icode = 0xA; ifun = 0x0;
            rA = parsear_registro(op1); rB = parsear_registro(op2);
        }
//nota: el enunciado menciona XORR como instrucción exclusiva pero no le asigna icode propio, se implementa como XORQ (icode 0xA, ifun 0x1) porque comparten la misma operación lógica
        else if (strcmp(mnemonico, "XORQ") == 0 || strcmp(mnemonico, "XORR") == 0) {
            icode = 0xA; ifun = 0x1;
            rA = parsear_registro(op1); rB = parsear_registro(op2);
        }
        else if (strcmp(mnemonico, "PUSH") == 0) {
            icode = 0x8; ifun = 0x0;
            rA = parsear_registro(op1);
        }
        else if (strcmp(mnemonico, "POP") == 0) {
            icode = 0x9; ifun = 0x0;
            rA = parsear_registro(op1);
        }
//nota: el enunciado no especifica icode para LDD ni MRMOVQ, se asigna 0x6 porque no colisiona con la tabla oficial del PDF
        else if (strcmp(mnemonico, "LDD") == 0 || strcmp(mnemonico, "MRMOVQ") == 0) {
            icode = 0x6; ifun = 0x0;
            rA = parsear_registro(op1);
            int32_t desp = 0;
            if (parsear_memoria(op2, &desp, &rB)) {
                valC = (uint64_t)(int64_t)desp;
            }
        }
//nota: el enunciado no especifica icode para STD ni RMMOVQ, se asigna 0x7 porque no colisiona con la tabla oficial del PDF
        else if (strcmp(mnemonico, "STD") == 0 || strcmp(mnemonico, "RMMOVQ") == 0) {
            icode = 0x7; ifun = 0x0;
            rA = parsear_registro(op1);
            int32_t desp = 0;
            if (parsear_memoria(op2, &desp, &rB)) {
                valC = (uint64_t)(int64_t)desp;
            }
        }
        else if (strcmp(mnemonico, "JMP") == 0) {
            icode = 0x3; ifun = 0x0;
            uint32_t dir = buscar_direccion_etiqueta(op1);
            if (dir != 0xFFFFFFFF)
                valC = (uint64_t)(int64_t)(dir - (pc_actual + 8));
        }
        else if (strcmp(mnemonico, "JL") == 0) {
            icode = 0x3; ifun = 0x1;
            uint32_t dir = buscar_direccion_etiqueta(op1);
            if (dir != 0xFFFFFFFF)
                valC = (uint64_t)(int64_t)(dir - (pc_actual + 8));
        }
        else if (strcmp(mnemonico, "JE") == 0) {
            icode = 0x3; ifun = 0x2;
            uint32_t dir = buscar_direccion_etiqueta(op1);
            if (dir != 0xFFFFFFFF)
                valC = (uint64_t)(int64_t)(dir - (pc_actual + 8));
        }
//nota: el enunciado no especifica ifun para JNZ.se extiende la familia JXX: 0x0=JMP, 0x1=JL, 0x2=JE, 0x3=JNZ
        else if (strcmp(mnemonico, "JNZ") == 0) {
            icode = 0x3; ifun = 0x3;
            uint32_t dir = buscar_direccion_etiqueta(op1);
            if (dir != 0xFFFFFFFF)
                valC = (uint64_t)(int64_t)(dir - (pc_actual + 8));
        }
        else {
            printf("Advertencia: Instruccion desconocida '%s'\n", mnemonico);
        }

        uint64_t codigo = 0;
        codigo |= ((uint64_t)(icode & 0xF)) << 60;
        codigo |= ((uint64_t)(ifun  & 0xF)) << 56;
        codigo |= ((uint64_t)(rA    & 0xF)) << 52;
        codigo |= ((uint64_t)(rB    & 0xF)) << 48;
        codigo |= (valC & 0xFFFFFFFFFFFFULL);

        fprintf(salida, "0x%08X | 0x%016llX | # %s\n",
                pc_actual,
                (unsigned long long)codigo,
                linea_original);

        pc_actual += 8;
    }

//registros finales:
    fprintf(salida, "\n--- registros finales ---\n");
    fprintf(salida, "R0:  0x0000000000000000\n");
    fprintf(salida, "R1:  0x0000000000000000\n");
    fprintf(salida, "R2:  0x0000000000000000\n");
    fprintf(salida, "R3:  0x0000000000000000\n");
    fprintf(salida, "R4:  0x0000000000000000\n");
    fprintf(salida, "R5:  0x0000000000000000\n");
    fprintf(salida, "R6:  0x0000000000000000 (SP)\n");
    fprintf(salida, "R7:  0x%016llX (PC)\n", (unsigned long long)pc_actual);
    fprintf(salida, "STAT: 0x0\n");

    fclose(entrada);
    fclose(salida);
    printf("Ensamblado exitoso, archivo %s generado.\n", archivo_txt);
}
