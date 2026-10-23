/* Utilidades generales: memoria, cadenas, listas de enteros y texto UTF-8. */
#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void *xmalloc(size_t n);
void *xcalloc(size_t cantidad, size_t n);
void *xrealloc(void *p, size_t n);
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);

/* Agranda una lista dinámica cuando ya está llena (duplica su capacidad).
 * Uso:  l->datos = crecer(l->datos, l->cantidad, &l->capacidad, sizeof *l->datos); */
void *crecer(void *datos, int cantidad, int *capacidad, size_t tam_elemento);

/* Lista dinámica de enteros (líneas de uso, historial de tipos, etc.) */
typedef struct {
    int *datos;
    int cantidad;
    int capacidad;
} ListaEnteros;

void enteros_agregar(ListaEnteros *l, int valor);
void enteros_liberar(ListaEnteros *l);

/* Cantidad de caracteres visibles de un texto UTF-8 ("año" = 3, no 4 bytes) */
size_t utf8_ancho(const char *s);

/* Agrega `texto` al final de `buf` sin pasarse de `n` bytes */
void agregar_texto(char *buf, size_t n, const char *texto);

#endif
