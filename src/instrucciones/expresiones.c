/* EXPRESIONES:  2 + x * 3,  "Hola, " + nombre,  edad >= 18
 *
 * Una expresión es todo lo que da un valor: un número, una variable, una
 * operación, una llamada a una chamba o un chorizo. Las usan todas las
 * instrucciones (contame <expresión>, si <expresión>...).
 */
#include "instrucciones/instrucciones.h"

#include <stdio.h>
#include <string.h>

#include "base/util.h"

/* ============================================================ sintaxis */

static Nodo *literal(const Token *t, TipoRP tipo, const char *lexema) {
    Nodo *n = nodo_nuevo(NODO_LITERAL, t->linea);
    n->lexema = xstrdup(lexema);
    n->literal.tipo = tipo;
    n->tipo = tipo;
    n->tiene_tipo = 1;
    return n;
}

/* primario → CABAL | CON_VUELTO | CHISME | 'simon' | 'nel' | 'ni' 'maiz'
 *          | IDENT | llamada | chorizo | 'cuantos' factor | '(' expresion ')' */
static Nodo *primario(Parser *p, const char *contexto) {
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
            if (siguiente(p)->tipo == TK_PAR_ABRE) return parsear_llamada(p);
            avanzar(p);
            return nodo_con_nombre(NODO_VARIABLE, t->linea, t->lexema);
        case TK_CORCHETE_ABRE:
            return parsear_chorizo(p);
        case TK_CUANTOS:
            return parsear_cuantos(p);
        case TK_PAR_ABRE:
            avanzar(p);
            n = parsear_expresion(p, "después de '('");
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
                            "se esperaba un valor %s (número, chisme, simon, nel, ni maiz, un chorizo, "
                            "una variable o una chamba), pero se encontró %s", contexto, describir(t, buf, sizeof buf));
    }
}

/* factor → primario { '[' expresion ']' }      (notas[0], tabla[1][2]) */
Nodo *parsear_factor(Parser *p, const char *contexto) {
    Nodo *n = primario(p, contexto);
    while (n && verificar(p, TK_CORCHETE_ABRE)) n = parsear_posicion(p, n);
    return n;
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
    Nodo *n = parsear_factor(p, contexto);
    while (n && (verificar(p, TK_POR) || verificar(p, TK_ENTRE))) n = operacion(p, n, parsear_factor);
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
Nodo *parsear_expresion(Parser *p, const char *contexto) {
    Nodo *n = suma(p, contexto);
    if (n && es_comparacion(actual(p)->tipo)) n = operacion(p, n, suma);
    return n;
}

/* ============================================================ semántica */

static TipoRP tipo_operacion_nodo(Semantico *sem, Nodo *nodo) {
    TipoRP izq = tipo_expresion(sem, nodo->izq);
    TipoRP der = tipo_expresion(sem, nodo->der);
    TipoRP resultado = RP_DESCONOCIDO;
    if (izq != RP_DESCONOCIDO && der != RP_DESCONOCIDO) {
        if (!tipo_operacion(nodo->operador, izq, der, &resultado)) {
            char mensaje[300];
            mensaje_operacion_invalida(nodo->operador, izq, der, mensaje, sizeof mensaje);
            error_semantico(sem, nodo->linea, "%s", mensaje);
            resultado = RP_DESCONOCIDO;
        }
    } else if (operador_es_comparacion(nodo->operador)) {
        /* Si un lado es dinámico la operación se revisa al ejecutar, pero una comparación siempre da simonel */
        resultado = RP_SIMONEL;
    }
    anotar(nodo, resultado);
    return resultado;
}

/* Inferencia de tipo: el literal ya trae su tipo; una variable tiene el de la tabla */
TipoRP tipo_expresion(Semantico *sem, Nodo *valor) {
    switch (valor->clase) {
        case NODO_LITERAL: return valor->tipo;
        case NODO_OPERACION: return tipo_operacion_nodo(sem, valor);
        case NODO_LLAMADA: return tipo_llamada(sem, valor);
        case NODO_CHORIZO: return tipo_chorizo(sem, valor);
        case NODO_POSICION: return tipo_posicion(sem, valor);
        case NODO_CUANTOS: return tipo_cuantos(sem, valor);
        default: break;
    }
    Simbolo *s = usar(sem, valor->nombre, valor->linea);
    /* Dentro de una chamba una variable global puede tener otro tipo cuando se llame la chamba */
    int dinamico = !s || (sem->chamba_actual && es_global(s));
    anotar(valor, dinamico ? RP_DESCONOCIDO : s->tipo);
    return valor->tipo;
}

/* ============================================================ ejecución */

static ValorRP evaluar_operacion(Interprete *in, const Nodo *nodo) {
    ValorRP izq = evaluar(in, nodo->izq);
    ValorRP der = in->hay_error ? NADA : evaluar(in, nodo->der);
    ValorRP resultado = NADA;
    char motivo[300];
    if (!in->hay_error && !valor_operar(nodo->operador, &izq, &der, &resultado, motivo, sizeof motivo))
        error_ejecucion(in, nodo->linea, "%s", motivo);
    valor_liberar(&izq);
    valor_liberar(&der);
    return resultado;
}

ValorRP evaluar(Interprete *in, const Nodo *valor) {
    switch (valor->clase) {
        case NODO_LITERAL: return valor_copiar(&valor->literal);
        case NODO_OPERACION: return evaluar_operacion(in, valor);
        case NODO_LLAMADA: return evaluar_llamada(in, valor);
        case NODO_CHORIZO: return evaluar_chorizo(in, valor);
        case NODO_POSICION: return evaluar_posicion(in, valor);
        case NODO_CUANTOS: return evaluar_cuantos(in, valor);
        default: break;
    }
    Celda *c = buscar_variable(in, valor->nombre);
    if (!c) {
        error_ejecucion(in, valor->linea, NO_EXISTE, valor->nombre);
        return NADA;
    }
    return valor_copiar(&c->valor);
}
