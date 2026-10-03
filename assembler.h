#ifndef assembler_h
#define assembler_h

#include <stdint.h>

// Cédulas del equipo:
#define cedula_1 33246671U   // Jesús Hernández
#define cedula_2 32110950U   // Erick Camargo
#define cedula_3 33432160U   // Farith Briceño

// Límites del ensamblador:
#define max_simbolos 200
#define max_etiqueta 50

// Estructura para la tabla de símbolos:
typedef struct {
    char etiqueta[max_etiqueta];
    uint32_t direccion;
} Simbolo;

//variables globales:
extern Simbolo tabla_simbolos[max_simbolos];
extern int total_simbolos;

uint32_t calcular_direccion_inicio(void);
uint32_t buscar_direccion_etiqueta(const char* nombre_etiqueta);
void primera_pasada(const char* archivo_asm, uint32_t direccion_inicio);
void segunda_pasada_y_generar_txt(const char* archivo_asm,const char* archivo_txt,uint32_t direccion_inicio);

#endif