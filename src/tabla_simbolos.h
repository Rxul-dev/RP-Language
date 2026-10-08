/* Tabla de símbolos implementada como tabla hash (dispersión con encadenamiento).
 *
 * RP tiene un solo ámbito (global), pero cada símbolo guarda su ámbito para que
 * la estructura pueda crecer cuando el lenguaje tenga funciones o bloques.
 */
#ifndef TABLA_SIMBOLOS_H
#define TABLA_SIMBOLOS_H

#include "util.h"
#include "valor.h"

#define TS_CUBETAS 64

typedef struct Simbolo {
    char *nombre;
    TipoRP tipo;                 /* tipo actual: cambia con cada 'ponele' (tipado dinámico) */
    const char *ambito;
    int linea_declaracion;
    ListaEnteros asignaciones;   /* líneas con 'ponele ... a nombre' */
    ListaEnteros usos;           /* líneas donde se lee la variable */
    ListaEnteros historial;      /* tipos que ha tenido (valores de TipoRP) */
    struct Simbolo *siguiente;   /* siguiente símbolo en la misma cubeta */
} Simbolo;

typedef struct {
    Simbolo *cubetas[TS_CUBETAS];
    Simbolo **orden;             /* en orden de declaración, para mostrar la tabla */
    int cantidad;
    int capacidad;
} TablaSimbolos;

void ts_iniciar(TablaSimbolos *ts);
Simbolo *ts_declarar(TablaSimbolos *ts, const char *nombre, TipoRP tipo, int linea);
Simbolo *ts_buscar(const TablaSimbolos *ts, const char *nombre);
void ts_liberar(TablaSimbolos *ts);

#endif
