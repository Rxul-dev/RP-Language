/* ANALIZADOR SINTÁCTICO (fase 2)
 *
 * ¿Qué hace?
 *   Revisa que los tokens estén en un orden válido, como revisar que una
 *   oración tenga sentido gramatical. Por ejemplo 'ponele 20 a edad' está bien,
 *   pero 'ponele 20 edad' no (falta la 'a').
 *
 *   Mientras revisa, arma el AST: un árbol que representa el programa.
 *
 * ¿Cómo está ordenado?
 *   Cada regla de la gramática (está completa en sintactico.h) es una función
 *   con el mismo nombre: declaracion(), asignacion(), chamba(), expresion()...
 *   Una función llama a otras: chamba() llama a linea() para cada línea del cuerpo.
 */
#include "sintactico.h"

#include <stdio.h>
#include <string.h>

#include "util.h"

#define INICIO_SENTENCIA \
    "las sentencias empiezan con 'fijate que', 'ponele', 'que onda', 'contame', 'si', 'chamba' o el nombre de una chamba"

typedef struct {
    const ListaTokens *tokens;
    int pos;
    ListaErrores *errores;
    const Token *chamba_abierta;  /* el nombre de la chamba cuyo cuerpo se está leyendo, o NULL */
    int si_abiertos;              /* cuántos 'si' se están leyendo, uno dentro de otro */
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

/* Modo pánico para un bloque (chamba o si) con el encabezado mal escrito: se descarta todo
 * hasta su 'ya estuvo' para no reportar su cuerpo como errores sueltos. Se cuentan los 'si'
 * de adentro para no confundir su 'ya estuvo' con el del bloque. */
static void saltar_bloque(Parser *p) {
    int abiertos = 1;
    for (;;) {
        saltar_linea(p);
        if (verificar(p, TK_FIN)) return;
        avanzar(p);  /* el salto de línea */
        if (verificar(p, TK_SI)) {
            abiertos++;
        } else if (verificar(p, TK_YA) && --abiertos == 0) {
            saltar_linea(p);
            return;
        } else if (verificar(p, TK_CHAMBA)) {
            p->pos--;  /* la siguiente chamba se analiza normalmente */
            return;
        }
    }
}

/* ------------------------------------------------------------ reglas */

/* `de_que` es "variable", "chamba" o "parámetro" */
static const Token *leer_nombre(Parser *p, const char *de_que, const char *contexto) {
    const Token *t = actual(p);
    char buf[200];
    if (t->tipo == TK_IDENT) return avanzar(p);
    if (token_es_reservada(t->tipo))
        return error_en(p, t, "'%s' es una palabra reservada y no puede usarse como nombre de %s", t->lexema, de_que);
    return error_en(p, t, "se esperaba el nombre de %s %s %s, pero se encontró %s",
                    strcmp(de_que, "parámetro") == 0 ? "un" : "una", de_que, contexto, describir(t, buf, sizeof buf));
}

static const Token *nombre_variable(Parser *p, const char *contexto) {
    return leer_nombre(p, "variable", contexto);
}

static Nodo *literal(const Token *t, TipoRP tipo, const char *lexema) {
    Nodo *n = nodo_nuevo(NODO_LITERAL, t->linea);
    n->lexema = xstrdup(lexema);
    n->literal.tipo = tipo;
    n->tipo = tipo;
    n->tiene_tipo = 1;
    return n;
}

static Nodo *expresion(Parser *p, const char *contexto);

/* llamada → IDENT '(' [ expresion { ',' expresion } ] ')' */
static Nodo *llamada(Parser *p) {
    const Token *nombre_chamba = avanzar(p);
    char buf[200], contexto[300];
    avanzar(p);  /* '(' */
    Nodo *n = nodo_nuevo(NODO_LLAMADA, nombre_chamba->linea);
    n->nombre = xstrdup(nombre_chamba->lexema);
    if (!verificar(p, TK_PAR_CIERRA)) {
        for (;;) {
            snprintf(contexto, sizeof contexto, "como argumento de '%s'", nombre_chamba->lexema);
            Nodo *arg = expresion(p, contexto);
            if (!arg) {
                ast_liberar(n);
                return NULL;
            }
            nodo_agregar_parametro(n, arg);
            if (!verificar(p, TK_COMA)) break;
            avanzar(p);
        }
    }
    if (!verificar(p, TK_PAR_CIERRA)) {
        error_en(p, actual(p), "falta ')' para cerrar la llamada a '%s' (se encontró %s); "
                 "los argumentos se separan con comas", nombre_chamba->lexema, describir(actual(p), buf, sizeof buf));
        ast_liberar(n);
        return NULL;
    }
    avanzar(p);
    return n;
}

/* factor → CABAL | CON_VUELTO | CHISME | 'simon' | 'nel' | 'ni' 'maiz' | IDENT | llamada | '(' expresion ')' */
static Nodo *factor(Parser *p, const char *contexto) {
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
            if (siguiente(p)->tipo == TK_PAR_ABRE) return llamada(p);
            avanzar(p);
            n = nodo_nuevo(NODO_VARIABLE, t->linea);
            n->nombre = xstrdup(t->lexema);
            return n;
        case TK_PAR_ABRE:
            avanzar(p);
            n = expresion(p, "después de '('");
            if (!n) return NULL;
            if (!verificar(p, TK_PAR_CIERRA)) {
                error_en(p, actual(p), "falta ')' para cerrar el paréntesis (se encontró %s)",
                         describir(actual(p), buf, sizeof buf));
                ast_liberar(n);
                return NULL;
            }
            avanzar(p);
            return n;
        case TK_MENOS:
            return error_en(p, t, "se esperaba un valor %s, pero se encontró un '-' suelto; "
                            "para un número negativo pegue el signo al número (ej. -5)", contexto);
        default:
            return error_en(p, t,
                            "se esperaba un valor %s (número, chisme, simon, nel, ni maiz, una variable "
                            "o una chamba), pero se encontró %s", contexto, describir(t, buf, sizeof buf));
    }
}

/* Construye una operación binaria con el operador actual; `siguiente_nivel` lee el lado derecho */
static Nodo *operacion(Parser *p, Nodo *izq, Nodo *(*siguiente_nivel)(Parser *, const char *)) {
    const Token *op = avanzar(p);
    char contexto[40];
    snprintf(contexto, sizeof contexto, "después de '%s'", op->lexema);
    Nodo *der = siguiente_nivel(p, contexto);
    if (!der) {
        ast_liberar(izq);
        return NULL;
    }
    Nodo *n = nodo_nuevo(NODO_OPERACION, op->linea);
    snprintf(n->operador, sizeof n->operador, "%s", op->lexema);
    n->izq = izq;
    n->der = der;
    return n;
}

/* termino → factor { ('*' | '/') factor } */
static Nodo *termino(Parser *p, const char *contexto) {
    Nodo *n = factor(p, contexto);
    while (n && (verificar(p, TK_POR) || verificar(p, TK_ENTRE))) n = operacion(p, n, factor);
    return n;
}

/* suma → termino { ('+' | '-') termino } */
static Nodo *suma(Parser *p, const char *contexto) {
    Nodo *n = termino(p, contexto);
    while (n && (verificar(p, TK_MAS) || verificar(p, TK_MENOS))) n = operacion(p, n, termino);
    return n;
}

static int es_comparacion(TipoToken tipo) {
    return tipo == TK_IGUAL || tipo == TK_DISTINTO || tipo == TK_MENOR || tipo == TK_MAYOR ||
           tipo == TK_MENOR_IGUAL || tipo == TK_MAYOR_IGUAL;
}

/* expresion → suma [ ('==' | '!=' | '<' | '>' | '<=' | '>=') suma ]
 * La comparación va al final: en 'x + 1 > 5' primero se suma y luego se compara.
 * Solo se permite una: '1 < x < 3' es un error. */
static Nodo *expresion(Parser *p, const char *contexto) {
    Nodo *n = suma(p, contexto);
    if (n && es_comparacion(actual(p)->tipo)) n = operacion(p, n, suma);
    return n;
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
        v = expresion(p, "después de 'es'");
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

    Nodo *v = expresion(p, "después de 'ponele'");
    if (!v) return NULL;

    if (!verificar(p, TK_A)) {
        const Token *t = actual(p);
        const char *texto = v->clase == NODO_LITERAL ? v->lexema : v->clase == NODO_VARIABLE ? v->nombre : "<valor>";
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

/* contame → 'contame' expresion */
static Nodo *contame(Parser *p) {
    const Token *inicio = avanzar(p);
    Nodo *v = expresion(p, "después de 'contame'");
    if (!v) return NULL;
    Nodo *n = nodo_nuevo(NODO_CONTAME, inicio->linea);
    n->valor = v;
    return n;
}

/* devolveme → 'devolveme' [ expresion ] */
static Nodo *devolveme(Parser *p) {
    const Token *inicio = avanzar(p);
    Nodo *v = NULL;
    if (!verificar(p, TK_SALTO) && !verificar(p, TK_FIN)) {
        v = expresion(p, "después de 'devolveme'");
        if (!v) return NULL;
    }
    Nodo *n = nodo_nuevo(NODO_DEVOLVEME, inicio->linea);
    n->valor = v;
    return n;
}

static void linea(Parser *p, Nodo *destino);

/* bloque → { linea } 'ya' 'estuvo'
 * El cuerpo de una chamba o de un si; sus sentencias se agregan a `destino`.
 * `que` describe el bloque para los mensajes ("la chamba 'sumar'"). Devuelve 0 si hubo error. */
static int bloque(Parser *p, Nodo *destino, const char *que) {
    char buf[200];
    for (;;) {
        if (verificar(p, TK_SALTO)) {
            avanzar(p);
        } else if (verificar(p, TK_FIN)) {
            error_en(p, actual(p), "%s no se cerró: falta 'ya estuvo' al final", que);
            return 0;
        } else if (verificar(p, TK_YA)) {
            avanzar(p);
            if (!verificar(p, TK_ESTUVO)) {
                error_en(p, actual(p), "se esperaba 'estuvo' después de 'ya': %s se cierra con 'ya estuvo' "
                         "(se encontró %s)", que, describir(actual(p), buf, sizeof buf));
                return 0;
            }
            avanzar(p);
            return 1;
        } else {
            linea(p, destino);
        }
    }
}

/* si → 'si' expresion SALTO bloque */
static Nodo *si(Parser *p) {
    const Token *inicio = avanzar(p);
    char buf[200], que[100];
    Nodo *condicion = expresion(p, "después de 'si'");
    if (!condicion) {
        saltar_bloque(p);
        return NULL;
    }
    if (!verificar(p, TK_SALTO)) {
        error_en(p, actual(p), "después de la condición del 'si' va un salto de línea; lo que se hace "
                 "se escribe en las líneas siguientes (sobra %s)", describir(actual(p), buf, sizeof buf));
        ast_liberar(condicion);
        saltar_bloque(p);
        return NULL;
    }
    Nodo *n = nodo_nuevo(NODO_SI, inicio->linea);
    n->valor = condicion;

    snprintf(que, sizeof que, "el 'si' de la línea %d", inicio->linea);
    p->si_abiertos++;
    int ok = bloque(p, n, que);
    p->si_abiertos--;
    if (!ok) {
        ast_liberar(n);
        return NULL;
    }
    return n;
}

/* chamba → 'chamba' IDENT '(' [ IDENT { ',' IDENT } ] ')' SALTO bloque */
static Nodo *chamba(Parser *p) {
    const Token *inicio = avanzar(p);
    char buf[200], que[100];
    if (p->chamba_abierta)
        return error_en(p, inicio, "no se puede declarar una chamba dentro de otra; "
                        "¿olvidó cerrar '%s' con 'ya estuvo'?", p->chamba_abierta->lexema);
    if (p->si_abiertos)
        return error_en(p, inicio, "las chambas se declaran afuera de los bloques 'si'; "
                        "¿olvidó cerrar el 'si' con 'ya estuvo'?");

    const Token *nombre_chamba = leer_nombre(p, "chamba", "después de 'chamba'");
    if (!nombre_chamba) {
        saltar_bloque(p);
        return NULL;
    }
    Nodo *n = nodo_nuevo(NODO_CHAMBA, inicio->linea);
    n->nombre = xstrdup(nombre_chamba->lexema);

    if (!verificar(p, TK_PAR_ABRE)) {
        error_en(p, actual(p), "falta '(' después del nombre: escriba 'chamba %s(<parámetros>)' "
                 "(se encontró %s)", nombre_chamba->lexema, describir(actual(p), buf, sizeof buf));
        goto error;
    }
    avanzar(p);
    if (!verificar(p, TK_PAR_CIERRA)) {
        for (;;) {
            const Token *param = leer_nombre(p, "parámetro", "en la chamba");
            if (!param) goto error;
            Nodo *np = nodo_nuevo(NODO_PARAMETRO, param->linea);
            np->nombre = xstrdup(param->lexema);
            nodo_agregar_parametro(n, np);
            if (!verificar(p, TK_COMA)) break;
            avanzar(p);
        }
    }
    if (!verificar(p, TK_PAR_CIERRA)) {
        error_en(p, actual(p), "falta ')' para cerrar los parámetros de '%s' (se encontró %s); "
                 "los parámetros se separan con comas", nombre_chamba->lexema, describir(actual(p), buf, sizeof buf));
        goto error;
    }
    avanzar(p);
    if (!verificar(p, TK_SALTO)) {
        error_en(p, actual(p), "después de 'chamba %s(...)' va un salto de línea; el cuerpo se escribe "
                 "en las líneas siguientes (sobra %s)", nombre_chamba->lexema, describir(actual(p), buf, sizeof buf));
        goto error;
    }

    snprintf(que, sizeof que, "la chamba '%s'", nombre_chamba->lexema);
    p->chamba_abierta = nombre_chamba;
    int ok = bloque(p, n, que);
    p->chamba_abierta = NULL;
    if (ok) return n;
    ast_liberar(n);
    return NULL;

error:
    ast_liberar(n);
    saltar_bloque(p);
    return NULL;
}

/* sentencia → declaracion | asignacion | consulta | contame | devolveme | si | chamba | llamada */
static Nodo *sentencia(Parser *p) {
    const Token *t = actual(p);
    char buf[200];
    switch (t->tipo) {
        case TK_FIJATE: return declaracion(p);
        case TK_PONELE: return asignacion(p);
        case TK_QUE: return consulta_tipo(p);
        case TK_CONTAME: return contame(p);
        case TK_DEVOLVEME: return devolveme(p);
        case TK_SI: return si(p);
        case TK_CHAMBA: return chamba(p);
        case TK_YA:
            return error_en(p, t, "'ya estuvo' cierra una chamba o un 'si', pero no hay ninguno abierto");
        default: break;
    }
    if (t->tipo == TK_IDENT && siguiente(p)->tipo == TK_PAR_ABRE) return llamada(p);
    if (t->tipo == TK_IDENT && siguiente(p)->tipo == TK_ES)
        return error_en(p, t,
                        "falta 'fijate que': para declarar escriba 'fijate que %s es ...' "
                        "y para cambiar su valor 'ponele <valor> a %s'", t->lexema, t->lexema);
    return error_en(p, t, "no se esperaba %s al inicio; " INICIO_SENTENCIA, describir(t, buf, sizeof buf));
}

/* linea → sentencia SALTO   (la sentencia se agrega a `destino`: el programa, una chamba o un si) */
static void linea(Parser *p, Nodo *destino) {
    char buf[200];
    /* El léxico ya reportó esta línea: no se agregan errores en cascada */
    if (linea_con_error_lexico(p)) {
        if (verificar(p, TK_SI) || (verificar(p, TK_CHAMBA) && !p->chamba_abierta))
            saltar_bloque(p);
        else
            saltar_linea(p);
        return;
    }
    Nodo *s = sentencia(p);
    if (s && !verificar(p, TK_SALTO)) {
        error_en(p, actual(p), "se esperaba el fin de la línea, pero sobra %s", describir(actual(p), buf, sizeof buf));
        ast_liberar(s);
        s = NULL;
    }
    if (s)
        nodo_agregar_sentencia(destino, s);
    else
        saltar_linea(p);  /* modo pánico */
}

/* programa → { linea } FIN */
Nodo *sintactico_analizar(const ListaTokens *tokens, ListaErrores *errores) {
    Parser p = {tokens, 0, errores, NULL, 0};
    Nodo *programa = nodo_nuevo(NODO_PROGRAMA, 1);

    while (!verificar(&p, TK_FIN)) {
        if (verificar(&p, TK_SALTO))
            avanzar(&p);
        else
            linea(&p, programa);
    }
    return programa;
}
