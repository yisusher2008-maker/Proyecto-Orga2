#ifndef LOGGER_H //Esto es porque si ya lo definieron antes en otro archivo, no vaya a redefinir el logger
//osea pregunta si ya esta definido (No me acuerdo si lo definieron o no porque recien me levanto)
#define LOGGER_H

#include <stdio.h>
#include "cpu.h"

FILE* abrir_logger(const char *nombre_archivo);
void registrar_estado(FILE *log_file, const cpu *c);
void cerrar_logger(FILE *log_file);

#endif
