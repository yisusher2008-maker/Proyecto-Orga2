#ifndef logger_h //Esto es porque si ya lo definieron antes en otro archivo, no vaya a redefinir el logger
//osea pregunta si ya esta definido (No me acuerdo si lo definieron o no porque recien me levanto)
#define logger_h

#include <stdio.h>
#include "cpu.h"

FILE* iniciar_log(const char *nombre_archivo);
void registrar_estado(FILE *log_file, const cpu *c);
void cerrar_log(FILE *log_file);

#endif
