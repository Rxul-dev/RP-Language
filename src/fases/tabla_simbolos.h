/* Tabla de símbolos implementada como tabla hash (dispersión con encadenamiento).
 *
 * Ámbitos: el global y uno por cada chamba (sus parámetros y variables locales).
 * Al buscar un nombre dentro de una chamba se mira primero su ámbito y luego el
 * global. Al salir de la chamba sus símbolos se quedan en la tabla (para el
 * reporte), pero ya no se encuentran desde afuera.
 */
#ifndef TABLA_SIMBOLOS_H
#define TABLA_SIMBOLOS_H

#include "base/util.h"
#include "base/valor.h"

#define TS_CUBETAS 64

typedef enum { SIM_VARIABLE, SIM_PARAMETRO, SIM_CHAMBA } ClaseSimbolo;

typedef struct Simbolo {
    char *nombre;
    ClaseSimbolo clase;
    TipoRP tipo;                 /* tipo actual: cambia con cada 'ponele' (tipado dinámico) */
    char *ambito;                /* "global" o "chamba <nombre>" */
    int num_parametros;          /* solo SIM_CHAMBA */
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
    char *ambito_actual;         /* NULL = global */
} TablaSimbolos;

void ts_iniciar(TablaSimbolos *ts);
/* Declara en el ámbito actual (como variable; cambie `clase` si es otra cosa) */
Simbolo *ts_declarar(TablaSimbolos *ts, const char *nombre, TipoRP tipo, int linea);
Simbolo *ts_buscar(const TablaSimbolos *ts, const char *nombre);            /* ámbito actual y luego global */
Simbolo *ts_buscar_en_ambito(const TablaSimbolos *ts, const char *nombre);  /* solo el ámbito actual */
void ts_entrar_chamba(TablaSimbolos *ts, const char *nombre);
void ts_salir_chamba(TablaSimbolos *ts);
const char *clase_simbolo_nombre(ClaseSimbolo clase);
void ts_liberar(TablaSimbolos *ts);

#endif
