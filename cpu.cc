#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cpu.h"
#include "logger.h"

/*extiende el signo de un valor de 48 bits a 64 bits
Necesario porque valC es de 48 bits (según el PDF) y puede
representar valores negativos (offsets de saltos hacia atrás
o desplazamientos negativos en LDD/STD)*/
static int64_t extender_signo_48(uint64_t val) {
    // Si el bit 47 está activo, el valor es negativo
    if (val & 0x800000000000ULL) {
        return (int64_t)(val | 0xFFFF000000000000ULL);
    }
    return (int64_t)val;
}

void inicializar_cpu(cpu *c, uint64_t direccion_inicio) {
    memset(c, 0, sizeof(cpu));
    c->pc = direccion_inicio;
    c->direccion_inicio = direccion_inicio;
    c->stat = stat_aok;
    c->registros[0] = 0; // R0 siempre es 0 de forma estricta
}

//actualiza las banderas ZF, SF y OF según el resultado de la operación:
static void actualizar_banderas(cpu *c, uint64_t resultado) {
    c->zf = (resultado == 0) ? 1 : 0;
    c->sf = (resultado & 1) ? 0 : 1; // 1 si es par (según PDF)
    c->of = 0;
}

//etapa de fetch:
static void fetch(cpu *c, instruccion *inst) {
    uint64_t offset = c->pc - c->direccion_inicio;

    if (offset % 8 != 0) {
        c->stat = stat_aex;
        return;
    }
    if (offset + 8 > tam_memoria) {
        c->stat = stat_adr;
        return;
    }

    uint64_t codigo = 0;
    for (int i = 0; i < 8; i++) {
        codigo |= ((uint64_t)c->memoria[offset + i]) << (i * 8);
    }

    inst->icode = (codigo >> 60) & 0xF;
    inst->ifun  = (codigo >> 56) & 0xF;
    inst->ra    = (codigo >> 52) & 0xF;
    inst->rb    = (codigo >> 48) & 0xF;
    inst->valc  = codigo & 0xFFFFFFFFFFFFULL;
}

//etapa de decode:
static void decode(cpu *c, instruccion *inst) {
    if (inst->icode == i_halt) return;

    switch (inst->icode) {
        case i_nop:
        case i_rrmvq:
        case i_irmovq:
        case i_opq:
        case i_jxx:
        case i_push:
        case i_pop:
        case i_ldd:
        case i_std:
            break;
        default:
            c->stat = stat_ins;
            return;
    }

    if (inst->icode == i_opq && inst->ifun > f_subq) {
        c->stat = stat_ins;
        return;
    }
    if (inst->icode == i_jxx && inst->ifun > f_jnz) {
        c->stat = stat_ins;
        return;
    }
}
//etapa de execute:
static void execute(cpu *c, instruccion *inst) {
    uint64_t vala = c->registros[inst->ra];
    uint64_t valb = c->registros[inst->rb];
    uint64_t resultado = 0;

    switch (inst->icode) {
        case i_opq:
            switch (inst->ifun) {
                case f_addq: resultado = vala + valb; break;
                case f_subq: resultado = valb - vala; break;
                case f_andq: resultado = vala & valb; break;
                case f_xorq: resultado = vala ^ valb; break;
            }
            if (inst->rb != 0) c->registros[inst->rb] = resultado;
            actualizar_banderas(c, resultado);
            break;

        case i_irmovq:
            if (inst->rb != 0) c->registros[inst->rb] = inst->valc;
            break;

        case i_rrmvq:
            if (inst->rb != 0) c->registros[inst->rb] = vala;
            break;

        default:
            break;
    }
}

//etapa de memory:
static void memory(cpu *c, instruccion *inst) {
    uint64_t direccion;
    uint64_t offset;
    int64_t svalc;

    switch (inst->icode) {
        case i_ldd: // LDD / MRMOVQ
            svalc = extender_signo_48(inst->valc);
            direccion = c->registros[inst->rb] + (uint64_t)svalc;
            offset = direccion - c->direccion_inicio;

            if (offset % 8 != 0) { c->stat = stat_aex; return; }
            if (offset + 8 > tam_memoria) { c->stat = stat_adr; return; }

            uint64_t valor = 0;
            for (int i = 0; i < 8; i++) {
                valor |= ((uint64_t)c->memoria[offset + i]) << (i * 8);
            }
            if (inst->ra != 0) c->registros[inst->ra] = valor;
            break;

        case i_std: // STD / RMMOVQ
            svalc = extender_signo_48(inst->valc);
            direccion = c->registros[inst->rb] + (uint64_t)svalc;
            offset = direccion - c->direccion_inicio;

            if (offset % 8 != 0) { c->stat = stat_aex; return; }
            if (offset + 8 > tam_memoria) { c->stat = stat_adr; return; }

            uint64_t valor_guardar = c->registros[inst->ra];
            for (int i = 0; i < 8; i++) {
                c->memoria[offset + i] = (valor_guardar >> (i * 8)) & 0xFF;
            }
            break;

        case i_push:
            c->registros[6] -= 8; // R6 es el Stack Pointer (%rsp)
            if (c->registros[6] % 8 != 0) { c->stat = stat_aex; return; }

            direccion = c->registros[6];
            offset = direccion - c->direccion_inicio;

            if (offset + 8 > tam_memoria) { c->stat = stat_adr; return; }

            uint64_t valor_push = c->registros[inst->ra];
            for (int i = 0; i < 8; i++) {
                c->memoria[offset + i] = (valor_push >> (i * 8)) & 0xFF;
            }
            break;

        case i_pop:
            direccion = c->registros[6];
            offset = direccion - c->direccion_inicio;

            if (offset % 8 != 0) { c->stat = stat_aex; return; }
            if (offset + 8 > tam_memoria) { c->stat = stat_adr; return; }

            uint64_t valor_pop = 0;
            for (int i = 0; i < 8; i++) {
                valor_pop |= ((uint64_t)c->memoria[offset + i]) << (i * 8);
            }
            if (inst->ra != 0) c->registros[inst->ra] = valor_pop;
            c->registros[6] += 8;
            break;

        default:
            break;
    }
}

//etapa de write back:
static void write_back(cpu *c, instruccion *inst) {
    if (inst->icode == i_halt) {
        c->stat = stat_hlt;
        return;
    }

    if (inst->icode == i_jxx) {
        int tomar_salto = 0;
        switch (inst->ifun) {
            case f_jmp: tomar_salto = 1; break;
            case f_je:  tomar_salto = c->zf; break;
            case f_jl:  tomar_salto = c->sf; break;
            case f_jnz: tomar_salto = !c->zf; break;
        }
        if (tomar_salto) {
            int64_t svalc = extender_signo_48(inst->valc);
            c->pc = c->pc + 8 + (uint64_t)svalc;
        } else {
            c->pc += 8;
        }
    } else {
        c->pc += 8;
    }
}
//ejecuta una instrucción completa (fetch, decode, execute, memory, write back):
void ejecutar_instruccion(cpu *c, instruccion *inst) {
    fetch(c, inst);
    if (c->stat != stat_aok) return;

    decode(c, inst);
    if (c->stat != stat_aok) return;

    execute(c, inst);
    if (c->stat != stat_aok) return;

    memory(c, inst);
    if (c->stat != stat_aok) return;

    write_back(c, inst);

    //blindaje final: R0 siempre debe permanecer en 0
    c->registros[0] = 0;
}

//ciclo principal de la CPU: ejecuta instrucciones hasta que se detenga:
void ciclo_cpu(cpu *c, FILE *log_file) {
    instruccion inst;

    while (c->stat == stat_aok) {
        registrar_estado(log_file, c);
        ejecutar_instruccion(c, &inst);

        if (c->stat != stat_aok) {
            registrar_estado(log_file, c);
            break;
        }
    }
}

/*
Carga el código de máquina desde el .txt
Convierte cada dirección absoluta a offset.
Ignora encabezados, separadores y la sección de registros
finales. Solo procesa líneas con el formato:
0xDIRECCION | 0xCODIGO | # instruccion
*/
void cargar_memoria(cpu *c, const char *archivo_txt, uint64_t direccion_inicio) {
    FILE *f = fopen(archivo_txt, "r");
    if (!f) {
        printf("Error: No se pudo abrir %s\n", archivo_txt);
        return;
    }

    char linea[512];

    while (fgets(linea, sizeof(linea), f)) {
        // Detenerse si llegamos a la sección de registros finales
        if (strstr(linea, "REGISTROS FINALES") != NULL) break;

        uint32_t dir;
        unsigned long long codigo;

        // Solo procesar líneas que tengan el formato de instrucción
        if (sscanf(linea, "0x%X | 0x%llX", &dir, &codigo) == 2) {
            uint32_t offset = dir - (uint32_t)direccion_inicio;
            for (int i = 0; i < 8; i++) {
                if (offset + i < tam_memoria) {
                    c->memoria[offset + i] = (codigo >> (i * 8)) & 0xFF;
                }
            }
        }
    }

    fclose(f);
}