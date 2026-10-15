/* Errores que reporta cada fase del compilador. */
#ifndef ERRORES_H
#define ERRORES_H

#include <stdarg.h>
#include <stddef.h>

typedef enum { FASE_LEXICA, FASE_SINTACTICA, FASE_SEMANTICA, FASE_EJECUCION } Fase;

typedef struct {
    Fase fase;
    int linea;
    int columna;  /* 0 cuando el error no tiene una columna precisa */
    int orden;    /* para desempatar al ordenar */
    char mensaje[600];
} ErrorRP;

typedef struct {
    ErrorRP *datos;
    int cantidad;
    int capacidad;
} ListaErrores;

void errores_agregar(ListaErrores *l, Fase fase, int linea, int columna, const char *formato, ...);
void errores_agregarv(ListaErrores *l, Fase fase, int linea, int columna, const char *formato, va_list args);
void errores_unir(ListaErrores *destino, const ListaErrores *origen);
void errores_ordenar(ListaErrores *l);
void errores_liberar(ListaErrores *l);

const char *fase_nombre(Fase fase);
void error_formatear(const ErrorRP *e, char *buf, size_t n);

#endif
