/* ERRORES
 *
 * Guarda los errores que encuentra cada fase (con su línea y columna) y los
 * ordena para mostrarlos de arriba abajo.
 */
#include "base/errores.h"

#include <stdio.h>
#include <stdlib.h>

#include "base/util.h"

static ErrorRP *nuevo_error(ListaErrores *l) {
    l->datos = crecer(l->datos, l->cantidad, &l->capacidad, sizeof *l->datos);
    ErrorRP *e = &l->datos[l->cantidad];
    e->orden = l->cantidad++;
    return e;
}

void errores_agregarv(ListaErrores *l, Fase fase, int linea, int columna, const char *formato, va_list args) {
    ErrorRP *e = nuevo_error(l);
    e->fase = fase;
    e->linea = linea;
    e->columna = columna;
    vsnprintf(e->mensaje, sizeof e->mensaje, formato, args);
}

void errores_agregar(ListaErrores *l, Fase fase, int linea, int columna, const char *formato, ...) {
    va_list args;
    va_start(args, formato);
    errores_agregarv(l, fase, linea, columna, formato, args);
    va_end(args);
}

void errores_unir(ListaErrores *destino, const ListaErrores *origen) {
    for (int i = 0; i < origen->cantidad; i++) {
        ErrorRP *e = nuevo_error(destino);
        int orden = e->orden;
        *e = origen->datos[i];
        e->orden = orden;
    }
}

static int comparar(const void *a, const void *b) {
    const ErrorRP *x = a, *y = b;
    if (x->linea != y->linea) return x->linea - y->linea;
    if (x->columna != y->columna) return x->columna - y->columna;
    return x->orden - y->orden;
}

void errores_ordenar(ListaErrores *l) {
    if (l->cantidad > 1) qsort(l->datos, (size_t)l->cantidad, sizeof *l->datos, comparar);
}

void errores_liberar(ListaErrores *l) {
    free(l->datos);
    l->datos = NULL;
    l->cantidad = l->capacidad = 0;
}

const char *fase_nombre(Fase fase) {
    switch (fase) {
        case FASE_LEXICA: return "léxico";
        case FASE_SINTACTICA: return "sintáctico";
        case FASE_EJECUCION: return "de ejecución";
        default: return "semántico";
    }
}

void error_formatear(const ErrorRP *e, char *buf, size_t n) {
    if (e->columna > 0)
        snprintf(buf, n, "Error %s (línea %d, columna %d): %s", fase_nombre(e->fase), e->linea, e->columna, e->mensaje);
    else
        snprintf(buf, n, "Error %s (línea %d): %s", fase_nombre(e->fase), e->linea, e->mensaje);
}
