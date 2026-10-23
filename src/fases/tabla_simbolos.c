/* TABLA DE SÍMBOLOS
 *
 * ¿Qué hace?
 *   Guarda todo lo que tiene nombre en el programa (variables, parámetros y
 *   chambas) con su tipo, dónde se declaró y dónde se usó.
 *
 * ¿Cómo busca rápido?
 *   Con una tabla hash: el nombre se convierte en un número (con la función
 *   hash) y ese número dice en qué "cubeta" está guardado. Así no hay que
 *   revisar todos los nombres uno por uno.
 */
#include "fases/tabla_simbolos.h"

#include <stdlib.h>
#include <string.h>

#define GLOBAL "global"

/* Función hash djb2 */
static unsigned hash(const char *s) {
    unsigned h = 5381;
    for (; *s; s++) h = h * 33 + (unsigned char)*s;
    return h % TS_CUBETAS;
}

void ts_iniciar(TablaSimbolos *ts) {
    memset(ts, 0, sizeof *ts);
}

static const char *ambito_actual(const TablaSimbolos *ts) {
    return ts->ambito_actual ? ts->ambito_actual : GLOBAL;
}

Simbolo *ts_declarar(TablaSimbolos *ts, const char *nombre, TipoRP tipo, int linea) {
    Simbolo *s = xcalloc(1, sizeof *s);
    s->nombre = xstrdup(nombre);
    s->clase = SIM_VARIABLE;
    s->tipo = tipo;
    s->ambito = xstrdup(ambito_actual(ts));
    s->linea_declaracion = linea;
    enteros_agregar(&s->historial, (int)tipo);

    unsigned i = hash(nombre);
    s->siguiente = ts->cubetas[i];
    ts->cubetas[i] = s;

    ts->orden = crecer(ts->orden, ts->cantidad, &ts->capacidad, sizeof *ts->orden);
    ts->orden[ts->cantidad++] = s;
    return s;
}

static Simbolo *buscar_en(const TablaSimbolos *ts, const char *nombre, const char *ambito) {
    for (Simbolo *s = ts->cubetas[hash(nombre)]; s; s = s->siguiente) {
        if (strcmp(s->nombre, nombre) == 0 && strcmp(s->ambito, ambito) == 0) return s;
    }
    return NULL;
}

Simbolo *ts_buscar_en_ambito(const TablaSimbolos *ts, const char *nombre) {
    return buscar_en(ts, nombre, ambito_actual(ts));
}

Simbolo *ts_buscar(const TablaSimbolos *ts, const char *nombre) {
    Simbolo *s = ts_buscar_en_ambito(ts, nombre);
    return s ? s : buscar_en(ts, nombre, GLOBAL);
}

/* El ámbito se llama "chamba <nombre>"; con el espacio no puede confundirse con "global" */
void ts_entrar_chamba(TablaSimbolos *ts, const char *nombre) {
    free(ts->ambito_actual);
    ts->ambito_actual = xmalloc(strlen("chamba ") + strlen(nombre) + 1);
    strcpy(ts->ambito_actual, "chamba ");
    strcat(ts->ambito_actual, nombre);
}

void ts_salir_chamba(TablaSimbolos *ts) {
    free(ts->ambito_actual);
    ts->ambito_actual = NULL;
}

const char *clase_simbolo_nombre(ClaseSimbolo clase) {
    switch (clase) {
        case SIM_PARAMETRO: return "parámetro";
        case SIM_CHAMBA: return "chamba";
        default: return "variable";
    }
}

void ts_liberar(TablaSimbolos *ts) {
    for (int i = 0; i < ts->cantidad; i++) {
        Simbolo *s = ts->orden[i];
        free(s->nombre);
        free(s->ambito);
        enteros_liberar(&s->asignaciones);
        enteros_liberar(&s->usos);
        enteros_liberar(&s->historial);
        free(s);
    }
    free(ts->orden);
    free(ts->ambito_actual);
    memset(ts, 0, sizeof *ts);
}
