#include <stdio.h>
#include "logger.h"

//Por si acaso dejare notas en lo que creo que podrian saber o no para que les sirva pa la defensa

FILE* abrir_logger(const char *nombre_archivo) {
    FILE *f = fopen(nombre_archivo, "w"); //Abre el archivo en forma de escribir o "write"
  //Eso lo que hace es que si el archivo no existe lo crea y si ya existe borra el contenido y escribe
  // en el desde 0, eso para que no se mezclen los registros aunque no estoy seguro de si esta bien eso
  //Porque no se si querian un archivo con tooodos los registros, de ser el caso pues se abre con "a"
  //en vez de con la w y ya pero aja, eso depende de que me digan ustedes pq no recuerdo el enunciado
    if (!f) {
        printf("Error: No se pudo crear el archivo %s\n", nombre_archivo);
    }
    return f;
}

void registrar_estado(FILE *log_file, const cpu *c) { //Se pasan con el * porque en el caso del file
  //en c no se trabaja con el archivo directamente si no con el puntero que te dice si esta abierto
  //y en la cpu porque como la cpu es muy grande, si se llama a cada rato estas gastando memoria a lo
  //tonto haciendo una copia de cpu a cada rato
    if (!log_file || !c) return;

  //Por si acaso explicacion de las variables en el PC, los R y el Stat
  //0x prefijo estandar del hexadecimal
  //% indica que lo que sigue es para definir una variable
  //0 despues del % indica rellenar con 0 a la izquierda en vez de usar espacios
  // el 16 define el ancho minimo de la cifra. Cada byte hexadecimal ocupa 2 digitos y un entero de
  // 64 bits siempre necesitara 16 digitos hexadecimales (eso porque el entero tiene 8bytes pue)
  //ll (long long): no hay mucho que explicar, solo indica que va a imprimir un entero de 64 bits
  //X indica que las letras del hexadecimal se imprimiran en mayuscula
  fprintf(log_file, 
        "PC: 0x%016llX | "
        "R0: 0x%016llX R1: 0x%016llX R2: 0x%016llX R3: 0x%016llX "
        "R4: 0x%016llX R5: 0x%016llX R6: 0x%016llX R7: 0x%016llX | "
        "STAT: 0x%X\n",
        (unsigned long long)c->pc,
        (unsigned long long)c->registros[0],
        (unsigned long long)c->registros[1],
        (unsigned long long)c->registros[2],
        (unsigned long long)c->registros[3],
        (unsigned long long)c->registros[4],
        (unsigned long long)c->registros[5],
        (unsigned long long)c->registros[6],
        (unsigned long long)c->registros[7],
        c->stat
    );
    fflush(log_file); // Garantiza escritura inmediata por cada ciclo
}

void cerrar_logger(FILE *log_file) {
    if (log_file) {
        fclose(log_file);
    }
}
