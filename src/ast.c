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

void nodo_agregar_parametro(Nodo *nodo, Nodo *parametro) {
    if (nodo->num_parametros == nodo->capacidad_parametros) {
        nodo->capacidad_parametros = nodo->capacidad_parametros ? nodo->capacidad_parametros * 2 : 4;
        nodo->parametros = xrealloc(nodo->parametros, (size_t)nodo->capacidad_parametros * sizeof(Nodo *));
    }
    nodo->parametros[nodo->num_parametros++] = parametro;
}

void ast_liberar(Nodo *nodo) {
    if (!nodo) return;
    for (int i = 0; i < nodo->num_sentencias; i++) ast_liberar(nodo->sentencias[i]);
    free(nodo->sentencias);
    for (int i = 0; i < nodo->num_parametros; i++) ast_liberar(nodo->parametros[i]);
    free(nodo->parametros);
    ast_liberar(nodo->valor);
    ast_liberar(nodo->izq);
    ast_liberar(nodo->der);
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
        case NODO_OPERACION: return "Operacion";
        case NODO_CHAMBA: return "Chamba";
        case NODO_PARAMETRO: return "Parametro";
        case NODO_LLAMADA: return "Llamada";
        case NODO_DEVOLVEME: return "Devolveme";
        case NODO_CONTAME: return "Contame";
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
        case NODO_OPERACION:
            snprintf(buf, n, "operador=%c", nodo->operador);
            break;
        case NODO_CHAMBA:
            snprintf(buf, n, "nombre=%s, %d parámetros, %d sentencias", nodo->nombre, nodo->num_parametros,
                     nodo->num_sentencias);
            break;
        case NODO_LLAMADA:
            snprintf(buf, n, "chamba=%s, %d argumentos", nodo->nombre, nodo->num_parametros);
            break;
        case NODO_DEVOLVEME:
            snprintf(buf, n, "%s", nodo->valor ? "devuelve el valor" : "sin valor → ni maiz");
            break;
        case NODO_CONTAME:
            snprintf(buf, n, "muestra el valor");
            break;
        default: /* CONSULTA_TIPO, VARIABLE y PARAMETRO */
            snprintf(buf, n, "nombre=%s", nodo->nombre);
    }
}

/* Hijos de una chamba: primero los parámetros y luego el cuerpo */
int nodo_num_hijos(const Nodo *nodo) {
    switch (nodo->clase) {
        case NODO_PROGRAMA: return nodo->num_sentencias;
        case NODO_CHAMBA: return nodo->num_parametros + nodo->num_sentencias;
        case NODO_LLAMADA: return nodo->num_parametros;
        case NODO_OPERACION: return 2;
        default: return nodo->valor ? 1 : 0;
    }
}

Nodo *nodo_hijo(const Nodo *nodo, int i) {
    switch (nodo->clase) {
        case NODO_PROGRAMA: return nodo->sentencias[i];
        case NODO_CHAMBA:
            return i < nodo->num_parametros ? nodo->parametros[i] : nodo->sentencias[i - nodo->num_parametros];
        case NODO_LLAMADA: return nodo->parametros[i];
        case NODO_OPERACION: return i == 0 ? nodo->izq : nodo->der;
        default: return nodo->valor;
    }
}
