/* Tokens del lenguaje RP. */
#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    /* Palabras reservadas */
    TK_FIJATE, TK_QUE, TK_ES, TK_PONELE, TK_A, TK_SIMON, TK_NEL, TK_NI, TK_MAIZ, TK_ONDA,
    TK_CHAMBA, TK_YA, TK_ESTUVO, TK_DEVOLVEME, TK_CONTAME,
    /* Nombres y literales */
    TK_IDENT, TK_CABAL, TK_CON_VUELTO, TK_CHISME,
    /* Símbolos */
    TK_PAR_ABRE, TK_PAR_CIERRA, TK_COMA, TK_MAS, TK_MENOS, TK_POR, TK_ENTRE,
    /* Control */
    TK_SALTO,  /* fin de sentencia */
    TK_ERROR,  /* el léxico ya reportó un error en este punto */
    TK_FIN
} TipoToken;

typedef struct {
    TipoToken tipo;
    char *lexema;
    int linea;
    int columna;
    long long entero;  /* valor de un TK_CABAL */
    double decimal;    /* valor de un TK_CON_VUELTO */
} Token;

typedef struct {
    Token *datos;
    int cantidad;
    int capacidad;
} ListaTokens;

const char *token_nombre(TipoToken tipo);
const char *token_categoria(TipoToken tipo);
int token_es_reservada(TipoToken tipo);
TipoToken buscar_palabra_reservada(const char *lexema); /* TK_IDENT si no es reservada */
void tokens_liberar(ListaTokens *l);

#endif
