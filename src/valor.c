/* VALORES DE RP
 *
 * ¿Qué hace?
 *   Define cómo se guarda un valor de RP (un cabal, un chisme, simon...) y las
 *   operaciones + - * / entre valores.
 *
 *   Cada valor lleva su tipo pegado (una "unión etiquetada"): así una variable
 *   puede guardar un cabal y luego un chisme, igual que en Python.
 */
#include "valor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

const char *tipo_rp_nombre(TipoRP tipo) {
    switch (tipo) {
        case RP_CABAL: return "cabal";
        case RP_CON_VUELTO: return "con vuelto";
        case RP_CHISME: return "chisme";
        case RP_SIMONEL: return "simonel";
        case RP_NI_MAIZ: return "ni maiz";
        default: return "dinámico";
    }
}

ValorRP valor_copiar(const ValorRP *v) {
    ValorRP copia = *v;
    if (v->tipo == RP_CHISME) copia.dato.chisme = xstrdup(v->dato.chisme);
    return copia;
}

void valor_liberar(ValorRP *v) {
    if (v->tipo == RP_CHISME) {
        free(v->dato.chisme);
        v->dato.chisme = NULL;
    }
    v->tipo = RP_NI_MAIZ;
}

void valor_formatear(const ValorRP *v, char *buf, size_t n) {
    switch (v->tipo) {
        case RP_CABAL:
            snprintf(buf, n, "%lld", v->dato.cabal);
            break;
        case RP_CON_VUELTO:
            snprintf(buf, n, "%.15g", v->dato.con_vuelto);
            /* 12.0 se imprimiría "12"; se agrega ".0" para que siga viéndose decimal */
            if (!strpbrk(buf, ".en")) strncat(buf, ".0", n - strlen(buf) - 1);
            break;
        case RP_CHISME:
            snprintf(buf, n, "\"%s\"", v->dato.chisme);
            break;
        case RP_SIMONEL:
            snprintf(buf, n, "%s", v->dato.simonel ? "simon" : "nel");
            break;
        default:
            snprintf(buf, n, "ni maiz");
    }
}

void valor_texto(const ValorRP *v, char *buf, size_t n) {
    if (v->tipo == RP_CHISME)
        snprintf(buf, n, "%s", v->dato.chisme);
    else
        valor_formatear(v, buf, n);
}

static int es_numero(TipoRP t) {
    return t == RP_CABAL || t == RP_CON_VUELTO;
}

int tipo_operacion(char op, TipoRP a, TipoRP b, TipoRP *resultado) {
    if (op == '+' && (a == RP_CHISME || b == RP_CHISME)) {
        *resultado = RP_CHISME;
        return 1;
    }
    if (!es_numero(a) || !es_numero(b)) return 0;
    *resultado = a == RP_CABAL && b == RP_CABAL ? RP_CABAL : RP_CON_VUELTO;
    return 1;
}

static const char *con_articulo(TipoRP t) {
    switch (t) {
        case RP_CABAL: return "un cabal";
        case RP_CON_VUELTO: return "un con vuelto";
        case RP_CHISME: return "un chisme";
        case RP_SIMONEL: return "un simonel";
        default: return "ni maiz";
    }
}

void mensaje_operacion_invalida(char op, TipoRP a, TipoRP b, char *buf, size_t n) {
    snprintf(buf, n, "no se puede usar '%c' entre %s y %s; %s", op, con_articulo(a), con_articulo(b),
             op == '+' ? "'+' solo suma números o une un chisme con otro valor"
                       : "esta operación solo funciona con números (cabal o con vuelto)");
}

static double como_decimal(const ValorRP *v) {
    return v->tipo == RP_CABAL ? (double)v->dato.cabal : v->dato.con_vuelto;
}

int valor_operar(char op, const ValorRP *a, const ValorRP *b, ValorRP *resultado, char *error, size_t n) {
    TipoRP tipo;
    if (!tipo_operacion(op, a->tipo, b->tipo, &tipo)) {
        mensaje_operacion_invalida(op, a->tipo, b->tipo, error, n);
        return 0;
    }
    resultado->tipo = tipo;

    if (tipo == RP_CHISME) {
        char texto_a[512], texto_b[512];
        const char *ta = a->tipo == RP_CHISME ? a->dato.chisme : texto_a;
        const char *tb = b->tipo == RP_CHISME ? b->dato.chisme : texto_b;
        valor_texto(a, texto_a, sizeof texto_a);
        valor_texto(b, texto_b, sizeof texto_b);
        size_t la = strlen(ta), lb = strlen(tb);
        char *unido = xmalloc(la + lb + 1);
        memcpy(unido, ta, la);
        memcpy(unido + la, tb, lb + 1);
        resultado->dato.chisme = unido;
        return 1;
    }

    if (op == '/' && como_decimal(b) == 0) {
        snprintf(error, n, "división entre cero");
        resultado->tipo = RP_NI_MAIZ;
        return 0;
    }
    if (tipo == RP_CABAL) {
        long long x = a->dato.cabal, y = b->dato.cabal;
        switch (op) {
            case '+': resultado->dato.cabal = x + y; break;
            case '-': resultado->dato.cabal = x - y; break;
            case '*': resultado->dato.cabal = x * y; break;
            default: resultado->dato.cabal = x / y;
        }
    } else {
        double x = como_decimal(a), y = como_decimal(b);
        switch (op) {
            case '+': resultado->dato.con_vuelto = x + y; break;
            case '-': resultado->dato.con_vuelto = x - y; break;
            case '*': resultado->dato.con_vuelto = x * y; break;
            default: resultado->dato.con_vuelto = x / y;
        }
    }
    return 1;
}
