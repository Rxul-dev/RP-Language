/* Tipos de dato de RP y la representación de un valor en memoria.
 *
 * C tiene tipado estático, así que el tipado dinámico de RP se implementa con
 * una "unión etiquetada": cada valor lleva consigo su tipo (campo `tipo`).
 */
#ifndef VALOR_H
#define VALOR_H

#include <stddef.h>

typedef enum {
    RP_CABAL,       /* entero:   42, -7       (int en C)    */
    RP_CON_VUELTO,  /* decimal:  3.75, -0.5   (double en C) */
    RP_CHISME,      /* texto:    "Ana"        (char* en C)  */
    RP_SIMONEL,     /* lógico:   simon, nel   (bool en C)   */
    RP_NI_MAIZ,     /* sin valor: ni maiz     (NULL en C)   */
    RP_DESCONOCIDO  /* el tipo se sabe hasta ejecutar (parámetros, resultado de una chamba)
                       o no se pudo calcular por un error semántico */
} TipoRP;

typedef struct {
    TipoRP tipo;
    union {
        long long cabal;
        double con_vuelto;
        char *chisme;
        int simonel;
    } dato;
} ValorRP;

const char *tipo_rp_nombre(TipoRP tipo);
ValorRP valor_copiar(const ValorRP *v);
void valor_liberar(ValorRP *v);
void valor_formatear(const ValorRP *v, char *buf, size_t n);  /* el chisme va entre comillas */
void valor_texto(const ValorRP *v, char *buf, size_t n);      /* como lo muestra 'contame' */

/* Operaciones aritméticas ('+', '-', '*', '/'):
 *   cabal  op cabal       → cabal (la división es entera: 7 / 2 = 3)
 *   números mezclados     → con vuelto
 *   chisme + cualquiera   → chisme (une los textos: "Edad: " + 19 = "Edad: 19")
 * Cualquier otra combinación es un error. */
int tipo_operacion(const char *op, TipoRP a, TipoRP b, TipoRP *resultado);  /* 0 si no es válida */
void mensaje_operacion_invalida(const char *op, TipoRP a, TipoRP b, char *buf, size_t n);
/* Calcula a op b; si no se puede, devuelve 0 y deja el motivo en `error` */
int valor_operar(const char *op, const ValorRP *a, const ValorRP *b, ValorRP *resultado, char *error, size_t n);

/* Comparaciones ('==', '!=', '<', '>', '<=', '>='): el resultado siempre es simonel.
 *   == y !=            → con cualquier par de valores (de tipos distintos nunca son iguales,
 *                        salvo cabal y con vuelto: 2 == 2.0 es simon)
 *   < > <= >=          → solo entre números */
int operador_es_comparacion(const char *op);

#endif
