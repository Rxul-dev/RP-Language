/* TOKENS
 *
 * Nombres y categorías de los tokens, para mostrarlos en la tabla de tokens
 * (./rp archivo.rp -f). Las palabras reservadas y los símbolos que reconoce RP
 * están en las tablas al inicio de lexico.c.
 */
#include "token.h"

#include <stdlib.h>

/* El nombre de cada token, en el mismo orden que el enum TipoToken de token.h */
static const char *NOMBRES[] = {
    "FIJATE", "QUE", "ES", "PONELE", "A", "SIMON", "NEL", "NI", "MAIZ", "ONDA",
    "CHAMBA", "YA", "ESTUVO", "DEVOLVEME", "CONTAME",
    "IDENT", "CABAL", "CON_VUELTO", "CHISME",
    "PAR_ABRE", "PAR_CIERRA", "COMA", "MAS", "MENOS", "POR", "ENTRE",
    "SALTO", "ERROR", "FIN",
};

const char *token_nombre(TipoToken tipo) {
    return NOMBRES[tipo];
}

const char *token_categoria(TipoToken tipo) {
    if (token_es_reservada(tipo)) return "palabra reservada";
    switch (tipo) {
        case TK_IDENT: return "identificador";
        case TK_CABAL: return "literal cabal";
        case TK_CON_VUELTO: return "literal con vuelto";
        case TK_CHISME: return "literal chisme";
        case TK_PAR_ABRE:
        case TK_PAR_CIERRA:
        case TK_COMA: return "signo de puntuación";
        case TK_MAS:
        case TK_MENOS:
        case TK_POR:
        case TK_ENTRE: return "operador aritmético";
        case TK_SALTO: return "fin de sentencia";
        case TK_ERROR: return "error léxico";
        default: return "fin de archivo";
    }
}

/* En el enum las palabras reservadas van primero: de TK_FIJATE a TK_CONTAME */
int token_es_reservada(TipoToken tipo) {
    return tipo <= TK_CONTAME;
}

void tokens_liberar(ListaTokens *l) {
    for (int i = 0; i < l->cantidad; i++) free(l->datos[i].lexema);
    free(l->datos);
    l->datos = NULL;
    l->cantidad = l->capacidad = 0;
}
