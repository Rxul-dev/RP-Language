#include "token.h"

#include <stdlib.h>
#include <string.h>

static const char *NOMBRES[] = {
    "FIJATE", "QUE", "ES", "PONELE", "A", "SIMON", "NEL", "NI", "MAIZ", "ONDA",
    "IDENT", "CABAL", "CON_VUELTO", "CHISME", "SALTO", "ERROR", "FIN",
};

/* Las palabras reservadas, en el mismo orden que TK_FIJATE..TK_ONDA */
static const char *RESERVADAS[] = {
    "fijate", "que", "es", "ponele", "a", "simon", "nel", "ni", "maiz", "onda",
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
        case TK_SALTO: return "fin de sentencia";
        case TK_ERROR: return "error léxico";
        default: return "fin de archivo";
    }
}

int token_es_reservada(TipoToken tipo) {
    return tipo <= TK_ONDA;
}

TipoToken buscar_palabra_reservada(const char *lexema) {
    for (int i = 0; i <= TK_ONDA; i++) {
        if (strcmp(lexema, RESERVADAS[i]) == 0) return (TipoToken)i;
    }
    return TK_IDENT;
}

void tokens_liberar(ListaTokens *l) {
    for (int i = 0; i < l->cantidad; i++) free(l->datos[i].lexema);
    free(l->datos);
    l->datos = NULL;
    l->cantidad = l->capacidad = 0;
}
