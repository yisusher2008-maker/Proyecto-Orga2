#ifndef cpu_h
#define cpu_h

#include <stdint.h>
#include <stdio.h>

//hardware base:
#define tam_memoria   4096
#define num_registros 8

//códigos de instrucción (icode):
#define i_halt   0xE
#define i_nop    0xF
#define i_rrmvq  0x5
#define i_irmovq 0x1
#define i_opq    0xA
#define i_jxx    0x3
#define i_push   0x8
#define i_pop    0x9
#define i_ldd    0x2   
#define i_std    0x4  

//funciones de la ALU:
#define f_andq 0x0
#define f_xorq 0x1
#define f_addq 0x2
#define f_subq 0x3

//funciones de salto:
#define f_jmp 0x0
#define f_jl  0x1
#define f_je  0x2
#define f_jnz 0x3


// Estados del sistema (STAT)
//   AOK = 0x0  (estado normal)
//   HLT = 0x1  (apagado normal vía HALT)
//   INS = 0x2  (instrucción ilegal)
//   AEX = 0x3  (error de alineación)
//   ADR = 0x4  (dirección inválida)

#define stat_aok 0x0
#define stat_hlt 0x1
#define stat_ins 0x2
#define stat_aex 0x3
#define stat_adr 0x4

//estructura de una instrucción decodificada:
typedef struct {
    uint8_t  icode;
    uint8_t  ifun;
    uint8_t  ra;
    uint8_t  rb;
    uint64_t valc;
} instruccion;

//estructura de la CPU:
typedef struct {
    uint64_t registros[num_registros]; // R0 a R7
    uint64_t pc;                       // Program Counter (dirección absoluta)
    uint64_t direccion_inicio;         // Dirección base del programa
    uint8_t  stat;                     // Registro de estado
    uint8_t  zf;                       // Zero Flag
    uint8_t  sf;                       // Sign Flag (según PDF: par = 1)
    uint8_t  of;                       // Overflow Flag
    uint8_t  memoria[tam_memoria];     // Memoria simulada (indexada por offset)
} cpu;

void inicializar_cpu(cpu *c, uint64_t direccion_inicio);
void ejecutar_instruccion(cpu *c, instruccion *inst);
void ciclo_cpu(cpu *c, FILE *log_file);
void cargar_memoria(cpu *c, const char *archivo_txt, uint64_t direccion_inicio);

#endif