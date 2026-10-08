#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

Nodo *nodo_nuevo(ClaseNodo clase, int linea) {
    Nodo *n = xcalloc(1, sizeof *n);
    n->clase = clase;
    n->linea = linea;
    n->tipo = RP_DESCONOCIDO;
    n->literal.tipo = RP_NI_MAIZ;
    return n;
}

void nodo_agregar_sentencia(Nodo *programa, Nodo *sentencia) {
    if (programa->num_sentencias == programa->capacidad) {
        programa->capacidad = programa->capacidad ? programa->capacidad * 2 : 8;
        programa->sentencias = xrealloc(programa->sentencias, (size_t)programa->capacidad * sizeof(Nodo *));
    }
    programa->sentencias[programa->num_sentencias++] = sentencia;
}

void ast_liberar(Nodo *nodo) {
    if (!nodo) return;
    for (int i = 0; i < nodo->num_sentencias; i++) ast_liberar(nodo->sentencias[i]);
    free(nodo->sentencias);
    ast_liberar(nodo->valor);
    free(nodo->nombre);
    free(nodo->lexema);
    valor_liberar(&nodo->literal);
    free(nodo);
}

const char *nodo_etiqueta(const Nodo *nodo) {
    switch (nodo->clase) {
        case NODO_PROGRAMA: return "Programa";
        case NODO_DECLARACION: return "Declaracion";
        case NODO_ASIGNACION: return "Asignacion";
        case NODO_CONSULTA_TIPO: return "ConsultaTipo";
        case NODO_LITERAL: return "Literal";
        default: return "Variable";
    }
}

void nodo_atributos(const Nodo *nodo, char *buf, size_t n) {
    switch (nodo->clase) {
        case NODO_PROGRAMA:
            snprintf(buf, n, "%d sentencias", nodo->num_sentencias);
            break;
        case NODO_DECLARACION:
            if (nodo->valor)
                snprintf(buf, n, "nombre=%s", nodo->nombre);
            else
                snprintf(buf, n, "nombre=%s, valor implícito=ni maiz", nodo->nombre);
            break;
        case NODO_ASIGNACION:
            snprintf(buf, n, "destino=%s", nodo->nombre);
            break;
        case NODO_LITERAL:
            snprintf(buf, n, "%s", nodo->lexema);
            break;
        default: /* CONSULTA_TIPO y VARIABLE */
            snprintf(buf, n, "nombre=%s", nodo->nombre);
    }
}

int nodo_num_hijos(const Nodo *nodo) {
    if (nodo->clase == NODO_PROGRAMA) return nodo->num_sentencias;
    return nodo->valor ? 1 : 0;
}

Nodo *nodo_hijo(const Nodo *nodo, int i) {
    return nodo->clase == NODO_PROGRAMA ? nodo->sentencias[i] : nodo->valor;
}
