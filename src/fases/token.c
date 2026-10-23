/* TOKENS
 *
 * Nombres y categorías de los tokens, para mostrarlos en la tabla de tokens
 * (./rp archivo.rp -f). Las palabras reservadas y los símbolos que reconoce RP
 * están en las tablas al inicio de lexico.c.
 */
#include "fases/token.h"

#include <stdlib.h>

/* El nombre de cada token; [TK_X] = "X" lo pone en la posición de su token aunque se cambie el orden */
static const char *NOMBRES[] = {
    [TK_FIJATE] = "FIJATE", [TK_QUE] = "QUE", [TK_ES] = "ES", [TK_PONELE] = "PONELE", [TK_A] = "A",
    [TK_SIMON] = "SIMON", [TK_NEL] = "NEL", [TK_NI] = "NI", [TK_MAIZ] = "MAIZ", [TK_ONDA] = "ONDA",
    [TK_CHAMBA] = "CHAMBA", [TK_YA] = "YA", [TK_ESTUVO] = "ESTUVO", [TK_DEVOLVEME] = "DEVOLVEME",
    [TK_SI] = "SI", [TK_CONTAME] = "CONTAME", [TK_METELE] = "METELE", [TK_CUANTOS] = "CUANTOS",
    [TK_IDENT] = "IDENT", [TK_CABAL] = "CABAL", [TK_CON_VUELTO] = "CON_VUELTO", [TK_CHISME] = "CHISME",
    [TK_PAR_ABRE] = "PAR_ABRE", [TK_PAR_CIERRA] = "PAR_CIERRA",
    [TK_CORCHETE_ABRE] = "CORCHETE_ABRE", [TK_CORCHETE_CIERRA] = "CORCHETE_CIERRA", [TK_COMA] = "COMA",
    [TK_MAS] = "MAS", [TK_MENOS] = "MENOS", [TK_POR] = "POR", [TK_ENTRE] = "ENTRE",
    [TK_IGUAL] = "IGUAL", [TK_DISTINTO] = "DISTINTO", [TK_MENOR] = "MENOR", [TK_MAYOR] = "MAYOR",
    [TK_MENOR_IGUAL] = "MENOR_IGUAL", [TK_MAYOR_IGUAL] = "MAYOR_IGUAL",
    [TK_SALTO] = "SALTO", [TK_ERROR] = "ERROR", [TK_FIN] = "FIN",
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
        case TK_CORCHETE_ABRE:
        case TK_CORCHETE_CIERRA:
        case TK_COMA: return "signo de puntuación";
        case TK_MAS:
        case TK_MENOS:
        case TK_POR:
        case TK_ENTRE: return "operador aritmético";
        case TK_IGUAL:
        case TK_DISTINTO:
        case TK_MENOR:
        case TK_MAYOR:
        case TK_MENOR_IGUAL:
        case TK_MAYOR_IGUAL: return "operador de comparación";
        case TK_SALTO: return "fin de sentencia";
        case TK_ERROR: return "error léxico";
        default: return "fin de archivo";
    }
}

/* En el enum las palabras reservadas van primero, antes de TK_IDENT */
int token_es_reservada(TipoToken tipo) {
    return tipo < TK_IDENT;
}

void tokens_liberar(ListaTokens *l) {
    for (int i = 0; i < l->cantidad; i++) free(l->datos[i].lexema);
    free(l->datos);
    l->datos = NULL;
    l->cantidad = l->capacidad = 0;
}
