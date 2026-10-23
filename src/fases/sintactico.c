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
 *   La regla de cada instrucción está en su archivo de src/instrucciones/
 *   (parsear_declaracion() en declaracion.c, parsear_si() en si.c...).
 *   Aquí queda lo que todas usan: moverse por los tokens, reportar errores,
 *   leer bloques y elegir qué instrucción empieza en cada línea.
 */
#include "fases/sintactico.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "instrucciones/instrucciones.h"

#define INICIO_SENTENCIA                                                                         \
    "las sentencias empiezan con 'fijate que', 'ponele', 'metele', 'que onda', 'contame', 'si', " \
    "'chamba' o el nombre de una chamba"

/* ------------------------------------------------------------ moverse por los tokens */

const Token *actual(Parser *p) {
    return &p->tokens->datos[p->pos];
}

const Token *siguiente(Parser *p) {
    int i = p->pos + 1 < p->tokens->cantidad ? p->pos + 1 : p->pos;
    return &p->tokens->datos[i];
}

const Token *avanzar(Parser *p) {
    const Token *t = actual(p);
    if (t->tipo != TK_FIN) p->pos++;
    return t;
}

int verificar(Parser *p, TipoToken tipo) {
    return actual(p)->tipo == tipo;
}

/* ------------------------------------------------------------ errores */

const char *describir(const Token *t, char *buf, size_t n) {
    if (t->tipo == TK_SALTO) return "el fin de la línea";
    if (t->tipo == TK_FIN) return "el fin del archivo";
    snprintf(buf, n, "'%s'", t->lexema);
    return buf;
}

void *error_en(Parser *p, const Token *t, const char *formato, ...) {
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

/* Se cuentan los 'si' de adentro para no confundir su 'ya estuvo' con el del bloque */
void saltar_bloque(Parser *p) {
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

/* ------------------------------------------------------------ reglas comunes */

const Token *leer_nombre(Parser *p, const char *de_que, const char *contexto) {
    const Token *t = actual(p);
    char buf[200];
    if (t->tipo == TK_IDENT) return avanzar(p);
    if (token_es_reservada(t->tipo))
        return error_en(p, t, "'%s' es una palabra reservada y no puede usarse como nombre de %s", t->lexema, de_que);
    return error_en(p, t, "se esperaba el nombre de %s %s %s, pero se encontró %s",
                    strcmp(de_que, "parámetro") == 0 ? "un" : "una", de_que, contexto, describir(t, buf, sizeof buf));
}

int leer_lista(Parser *p, Nodo *destino, TipoToken cierre, LeerElemento leer_uno, const char *que) {
    char buf[200], contexto[300];
    snprintf(contexto, sizeof contexto, "en %s", que);
    if (!verificar(p, cierre)) {
        for (;;) {
            Nodo *elemento = leer_uno(p, contexto);
            if (!elemento) return 0;
            nodo_agregar_elemento(destino, elemento);
            if (!verificar(p, TK_COMA)) break;
            avanzar(p);
        }
    }
    if (!verificar(p, cierre)) {
        error_en(p, actual(p), "falta '%s' para cerrar %s (se encontró %s); los valores se separan con comas",
                 cierre == TK_PAR_CIERRA ? ")" : "]", que, describir(actual(p), buf, sizeof buf));
        return 0;
    }
    avanzar(p);
    return 1;
}

Nodo *instruccion_con_valor(Parser *p, ClaseNodo clase) {
    const Token *inicio = avanzar(p);
    char contexto[100];
    snprintf(contexto, sizeof contexto, "después de '%s'", inicio->lexema);
    Nodo *v = parsear_expresion(p, contexto);
    if (!v) return NULL;
    Nodo *n = nodo_nuevo(clase, inicio->linea);
    n->valor = v;
    return n;
}

Nodo *instruccion_valor_a_nombre(Parser *p, ClaseNodo clase) {
    const Token *inicio = avanzar(p);
    const char *palabra = inicio->lexema;
    char buf[200], contexto[100];
    if (verificar(p, TK_A))
        return error_en(p, actual(p), "falta el valor: la forma es '%s <valor> a <nombre>' (el valor va antes "
                        "de la 'a')", palabra);

    snprintf(contexto, sizeof contexto, "después de '%s'", palabra);
    Nodo *v = parsear_expresion(p, contexto);
    if (!v) return NULL;

    const Token *t = actual(p);
    const Token *nombre = NULL;
    if (t->tipo == TK_A) {
        avanzar(p);
        nombre = leer_nombre(p, "variable", "después de 'a'");
    } else if (t->tipo == TK_IDENT) {
        const char *texto = v->clase == NODO_LITERAL ? v->lexema : v->clase == NODO_VARIABLE ? v->nombre : "<valor>";
        error_en(p, t, "falta la palabra 'a': escriba '%s %s a %s'", palabra, texto, t->lexema);
    } else {
        error_en(p, t, "se esperaba la palabra 'a' después del valor, pero se encontró %s", describir(t, buf, sizeof buf));
    }
    if (!nombre) {
        ast_liberar(v);
        return NULL;
    }
    Nodo *n = nodo_con_nombre(clase, inicio->linea, nombre->lexema);
    n->valor = v;
    return n;
}

/* sentencia → declaracion | asignacion | consulta | contame | devolveme | si | chamba | metele | llamada */
static Nodo *sentencia(Parser *p) {
    const Token *t = actual(p);
    char buf[200];
    switch (t->tipo) {
        case TK_FIJATE: return parsear_declaracion(p);
        case TK_PONELE: return parsear_asignacion(p);
        case TK_QUE: return parsear_consulta(p);
        case TK_CONTAME: return parsear_contame(p);
        case TK_DEVOLVEME: return parsear_devolveme(p);
        case TK_SI: return parsear_si(p);
        case TK_CHAMBA: return parsear_chamba(p);
        case TK_METELE: return parsear_metele(p);
        case TK_YA:
            return error_en(p, t, "'ya estuvo' cierra una chamba o un 'si', pero no hay ninguno abierto");
        default: break;
    }
    if (t->tipo == TK_IDENT && siguiente(p)->tipo == TK_PAR_ABRE) return parsear_llamada(p);
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

/* `que` describe el bloque para los mensajes ("la chamba 'sumar'") */
int bloque(Parser *p, Nodo *destino, const char *que) {
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
