#include "tabla_simbolos.h"

#include <stdlib.h>
#include <string.h>

/* Función hash djb2 */
static unsigned hash(const char *s) {
    unsigned h = 5381;
    for (; *s; s++) h = h * 33 + (unsigned char)*s;
    return h % TS_CUBETAS;
}

void ts_iniciar(TablaSimbolos *ts) {
    memset(ts, 0, sizeof *ts);
}

Simbolo *ts_declarar(TablaSimbolos *ts, const char *nombre, TipoRP tipo, int linea) {
    Simbolo *s = xcalloc(1, sizeof *s);
    s->nombre = xstrdup(nombre);
    s->tipo = tipo;
    s->ambito = "global";
    s->linea_declaracion = linea;
    enteros_agregar(&s->historial, (int)tipo);

    unsigned i = hash(nombre);
    s->siguiente = ts->cubetas[i];
    ts->cubetas[i] = s;

    if (ts->cantidad == ts->capacidad) {
        ts->capacidad = ts->capacidad ? ts->capacidad * 2 : 8;
        ts->orden = xrealloc(ts->orden, (size_t)ts->capacidad * sizeof *ts->orden);
    }
    ts->orden[ts->cantidad++] = s;
    return s;
}

Simbolo *ts_buscar(const TablaSimbolos *ts, const char *nombre) {
    for (Simbolo *s = ts->cubetas[hash(nombre)]; s; s = s->siguiente) {
        if (strcmp(s->nombre, nombre) == 0) return s;
    }
    return NULL;
}

void ts_liberar(TablaSimbolos *ts) {
    for (int i = 0; i < ts->cantidad; i++) {
        Simbolo *s = ts->orden[i];
        free(s->nombre);
        enteros_liberar(&s->asignaciones);
        enteros_liberar(&s->usos);
        enteros_liberar(&s->historial);
        free(s);
    }
    free(ts->orden);
    memset(ts, 0, sizeof *ts);
}
