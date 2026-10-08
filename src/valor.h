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
    RP_DESCONOCIDO  /* solo se usa al reportar errores semánticos */
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
void valor_formatear(const ValorRP *v, char *buf, size_t n);

#endif
