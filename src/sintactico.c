#include "sintactico.h"

#include <stdio.h>
#include <string.h>

#include "util.h"

#define INICIO_SENTENCIA "las sentencias empiezan con 'fijate que', 'ponele' o 'que onda'"

typedef struct {
    const ListaTokens *tokens;
    int pos;
    ListaErrores *errores;
} Parser;

/* ------------------------------------------------------------ utilidades */

static const Token *actual(Parser *p) {
    return &p->tokens->datos[p->pos];
}

static const Token *siguiente(Parser *p) {
    int i = p->pos + 1 < p->tokens->cantidad ? p->pos + 1 : p->pos;
    return &p->tokens->datos[i];
}

static const Token *avanzar(Parser *p) {
    const Token *t = actual(p);
    if (t->tipo != TK_FIN) p->pos++;
    return t;
}

static int verificar(Parser *p, TipoToken tipo) {
    return actual(p)->tipo == tipo;
}

/* Texto para los mensajes: 'edad', el fin de la línea, ... */
static const char *describir(const Token *t, char *buf, size_t n) {
    if (t->tipo == TK_SALTO) return "el fin de la línea";
    if (t->tipo == TK_FIN) return "el fin del archivo";
    snprintf(buf, n, "'%s'", t->lexema);
    return buf;
}

/* Reporta un error sintáctico en el token `t`; siempre devuelve NULL */
static void *error_en(Parser *p, const Token *t, const char *formato, ...) {
    va_list args;
    va_start(args, formato);
    errores_agregarv(p->errores, FASE_SINTACTICA, t->linea, t->columna, formato, args);
    va_end(args);
    return NULL;
}

static int linea_con_error_lexico(Parser *p) {
    for (int i = p->pos; p->tokens->datos[i].tipo != TK_SALTO && p->tokens->datos[i].tipo != TK_FIN; i++) {
        if (p->tokens->datos[i].tipo == TK_ERROR) return 1;
    }
    return 0;
}

static void saltar_linea(Parser *p) {
    while (!verificar(p, TK_SALTO) && !verificar(p, TK_FIN)) avanzar(p);
}

/* ------------------------------------------------------------ reglas */

static const Token *nombre_variable(Parser *p, const char *contexto) {
    const Token *t = actual(p);
    char buf[200];
    if (t->tipo == TK_IDENT) return avanzar(p);
    if (token_es_reservada(t->tipo))
        return error_en(p, t, "'%s' es una palabra reservada y no puede usarse como nombre de variable", t->lexema);
    return error_en(p, t, "se esperaba el nombre de una variable %s, pero se encontró %s",
                    contexto, describir(t, buf, sizeof buf));
}

static Nodo *literal(const Token *t, TipoRP tipo, const char *lexema) {
    Nodo *n = nodo_nuevo(NODO_LITERAL, t->linea);
    n->lexema = xstrdup(lexema);
    n->literal.tipo = tipo;
    n->tipo = tipo;
    n->tiene_tipo = 1;
    return n;
}

/* valor → CABAL | CON_VUELTO | CHISME | 'simon' | 'nel' | 'ni' 'maiz' | IDENT */
static Nodo *valor(Parser *p, const char *contexto) {
    const Token *t = actual(p);
    char buf[200];
    Nodo *n;
    switch (t->tipo) {
        case TK_CABAL:
            avanzar(p);
            n = literal(t, RP_CABAL, t->lexema);
            n->literal.dato.cabal = t->entero;
            return n;
        case TK_CON_VUELTO:
            avanzar(p);
            n = literal(t, RP_CON_VUELTO, t->lexema);
            n->literal.dato.con_vuelto = t->decimal;
            return n;
        case TK_CHISME:
            avanzar(p);
            n = literal(t, RP_CHISME, t->lexema);
            n->literal.dato.chisme = xstrndup(t->lexema + 1, strlen(t->lexema) - 2); /* sin comillas */
            return n;
        case TK_SIMON:
        case TK_NEL:
            avanzar(p);
            n = literal(t, RP_SIMONEL, t->lexema);
            n->literal.dato.simonel = t->tipo == TK_SIMON;
            return n;
        case TK_NI:
            avanzar(p);
            if (!verificar(p, TK_MAIZ))
                return error_en(p, actual(p),
                                "se esperaba 'maiz' después de 'ni' (el valor vacío se escribe 'ni maiz'), "
                                "pero se encontró %s", describir(actual(p), buf, sizeof buf));
            avanzar(p);
            return literal(t, RP_NI_MAIZ, "ni maiz");
        case TK_IDENT:
            avanzar(p);
            n = nodo_nuevo(NODO_VARIABLE, t->linea);
            n->nombre = xstrdup(t->lexema);
            return n;
        default:
            return error_en(p, t,
                            "se esperaba un valor %s (número, chisme, simon, nel, ni maiz o una variable), "
                            "pero se encontró %s", contexto, describir(t, buf, sizeof buf));
    }
}

/* declaracion → 'fijate' 'que' IDENT [ 'es' valor ] */
static Nodo *declaracion(Parser *p) {
    const Token *inicio = avanzar(p);
    char buf[200];
    if (!verificar(p, TK_QUE))
        return error_en(p, actual(p), "falta 'que': la frase completa para declarar es 'fijate que' (se encontró %s)",
                        describir(actual(p), buf, sizeof buf));
    avanzar(p);

    const Token *nombre = nombre_variable(p, "después de 'fijate que'");
    if (!nombre) return NULL;

    Nodo *v = NULL;
    if (verificar(p, TK_ES)) {
        avanzar(p);
        v = valor(p, "después de 'es'");
        if (!v) return NULL;
    } else if (verificar(p, TK_IDENT)) {
        const char *extra = actual(p)->lexema;
        return error_en(p, actual(p),
                        "el nombre de una variable no puede tener espacios ('%s %s'); use por ejemplo '%s_%s'",
                        nombre->lexema, extra, nombre->lexema, extra);
    }

    Nodo *n = nodo_nuevo(NODO_DECLARACION, inicio->linea);
    n->nombre = xstrdup(nombre->lexema);
    n->valor = v;
    return n;
}

/* asignacion → 'ponele' valor 'a' IDENT */
static Nodo *asignacion(Parser *p) {
    const Token *inicio = avanzar(p);
    char buf[200];
    if (verificar(p, TK_A))
        return error_en(p, actual(p),
                        "falta el valor: la forma es 'ponele <valor> a <nombre>' (el valor va antes de la 'a')");

    Nodo *v = valor(p, "después de 'ponele'");
    if (!v) return NULL;

    if (!verificar(p, TK_A)) {
        const Token *t = actual(p);
        const char *texto = v->clase == NODO_LITERAL ? v->lexema : v->nombre;
        if (t->tipo == TK_IDENT)
            error_en(p, t, "falta la palabra 'a': escriba 'ponele %s a %s'", texto, t->lexema);
        else
            error_en(p, t, "se esperaba la palabra 'a' después del valor, pero se encontró %s",
                     describir(t, buf, sizeof buf));
        ast_liberar(v);
        return NULL;
    }
    avanzar(p);

    const Token *nombre = nombre_variable(p, "después de 'a'");
    if (!nombre) {
        ast_liberar(v);
        return NULL;
    }
    Nodo *n = nodo_nuevo(NODO_ASIGNACION, inicio->linea);
    n->nombre = xstrdup(nombre->lexema);
    n->valor = v;
    return n;
}

/* consulta → 'que' 'onda' IDENT */
static Nodo *consulta_tipo(Parser *p) {
    const Token *inicio = avanzar(p);
    char buf[200];
    if (!verificar(p, TK_ONDA))
        return error_en(p, actual(p),
                        "se esperaba 'onda' después de 'que'; para ver el tipo use 'que onda <nombre>' "
                        "(se encontró %s)", describir(actual(p), buf, sizeof buf));
    avanzar(p);

    const Token *nombre = nombre_variable(p, "después de 'que onda'");
    if (!nombre) return NULL;
    Nodo *n = nodo_nuevo(NODO_CONSULTA_TIPO, inicio->linea);
    n->nombre = xstrdup(nombre->lexema);
    return n;
}

/* sentencia → declaracion | asignacion | consulta */
static Nodo *sentencia(Parser *p) {
    const Token *t = actual(p);
    char buf[200];
    switch (t->tipo) {
        case TK_FIJATE: return declaracion(p);
        case TK_PONELE: return asignacion(p);
        case TK_QUE: return consulta_tipo(p);
        default: break;
    }
    if (t->tipo == TK_IDENT && siguiente(p)->tipo == TK_ES)
        return error_en(p, t,
                        "falta 'fijate que': para declarar escriba 'fijate que %s es ...' "
                        "y para cambiar su valor 'ponele <valor> a %s'", t->lexema, t->lexema);
    return error_en(p, t, "no se esperaba %s al inicio; " INICIO_SENTENCIA, describir(t, buf, sizeof buf));
}

/* programa → { linea } FIN */
Nodo *sintactico_analizar(const ListaTokens *tokens, ListaErrores *errores) {
    Parser p = {tokens, 0, errores};
    Nodo *programa = nodo_nuevo(NODO_PROGRAMA, 1);
    char buf[200];

    while (!verificar(&p, TK_FIN)) {
        if (verificar(&p, TK_SALTO)) {
            avanzar(&p);
            continue;
        }
        /* El léxico ya reportó esta línea: no se agregan errores en cascada */
        if (linea_con_error_lexico(&p)) {
            saltar_linea(&p);
            continue;
        }
        Nodo *s = sentencia(&p);
        if (s && !verificar(&p, TK_SALTO)) {
            error_en(&p, actual(&p), "se esperaba el fin de la línea, pero sobra %s",
                     describir(actual(&p), buf, sizeof buf));
            ast_liberar(s);
            s = NULL;
        }
        if (s)
            nodo_agregar_sentencia(programa, s);
        else
            saltar_linea(&p);  /* modo pánico */
    }
    return programa;
}
