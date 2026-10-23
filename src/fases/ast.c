/* AST (árbol de sintaxis abstracta)
 *
 * ¿Qué es?
 *   El programa convertido en árbol. Por ejemplo 'contame 2 + 3' queda así:
 *
 *       Contame
 *       └── Operacion (+)
 *           ├── Literal (2)
 *           └── Literal (3)
 *
 *   Aquí están las funciones para crear nodos, liberarlos y mostrarlos.
 */
#include "fases/ast.h"

#include <stdio.h>
#include <stdlib.h>

#include "base/util.h"

Nodo *nodo_nuevo(ClaseNodo clase, int linea) {
    Nodo *n = xcalloc(1, sizeof *n);
    n->clase = clase;
    n->linea = linea;
    n->tipo = RP_DESCONOCIDO;
    n->literal.tipo = RP_NI_MAIZ;
    return n;
}

Nodo *nodo_con_nombre(ClaseNodo clase, int linea, const char *nombre) {
    Nodo *n = nodo_nuevo(clase, linea);
    n->nombre = xstrdup(nombre);
    return n;
}

void nodo_agregar_sentencia(Nodo *programa, Nodo *sentencia) {
    programa->sentencias = crecer(programa->sentencias, programa->num_sentencias, &programa->capacidad,
                                  sizeof(Nodo *));
    programa->sentencias[programa->num_sentencias++] = sentencia;
}

void nodo_agregar_elemento(Nodo *nodo, Nodo *elemento) {
    nodo->elementos = crecer(nodo->elementos, nodo->num_elementos, &nodo->capacidad_elementos, sizeof(Nodo *));
    nodo->elementos[nodo->num_elementos++] = elemento;
}

void ast_liberar(Nodo *nodo) {
    if (!nodo) return;
    for (int i = 0; i < nodo->num_sentencias; i++) ast_liberar(nodo->sentencias[i]);
    free(nodo->sentencias);
    for (int i = 0; i < nodo->num_elementos; i++) ast_liberar(nodo->elementos[i]);
    free(nodo->elementos);
    ast_liberar(nodo->valor);
    ast_liberar(nodo->posicion);
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
        case NODO_CONTAME: return "Contame";
        case NODO_SI: return "Si";
        case NODO_CHAMBA: return "Chamba";
        case NODO_PARAMETRO: return "Parametro";
        case NODO_DEVOLVEME: return "Devolveme";
        case NODO_METELE: return "Metele";
        case NODO_LITERAL: return "Literal";
        case NODO_OPERACION: return "Operacion";
        case NODO_LLAMADA: return "Llamada";
        case NODO_CHORIZO: return "Chorizo";
        case NODO_POSICION: return "Posicion";
        case NODO_CUANTOS: return "Cuantos";
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
            snprintf(buf, n, "destino=%s%s", nodo->nombre, nodo->posicion ? "[posición]" : "");
            break;
        case NODO_CONTAME:
            snprintf(buf, n, "muestra el valor");
            break;
        case NODO_SI:
            snprintf(buf, n, "condición y %d sentencias", nodo->num_sentencias);
            break;
        case NODO_CHAMBA:
            snprintf(buf, n, "nombre=%s, %d parámetros, %d sentencias", nodo->nombre, nodo->num_elementos,
                     nodo->num_sentencias);
            break;
        case NODO_DEVOLVEME:
            snprintf(buf, n, "%s", nodo->valor ? "devuelve el valor" : "sin valor → ni maiz");
            break;
        case NODO_METELE:
            snprintf(buf, n, "chorizo=%s", nodo->nombre);
            break;
        case NODO_LITERAL:
            snprintf(buf, n, "%s", nodo->lexema);
            break;
        case NODO_OPERACION:
            snprintf(buf, n, "operador=%s", nodo->operador);
            break;
        case NODO_LLAMADA:
            snprintf(buf, n, "chamba=%s, %d argumentos", nodo->nombre, nodo->num_elementos);
            break;
        case NODO_CHORIZO:
            snprintf(buf, n, "%d valores", nodo->num_elementos);
            break;
        case NODO_POSICION:
            snprintf(buf, n, "chorizo y posición");
            break;
        case NODO_CUANTOS:
            snprintf(buf, n, "cantidad de valores");
            break;
        default: /* CONSULTA_TIPO, VARIABLE y PARAMETRO */
            snprintf(buf, n, "nombre=%s", nodo->nombre);
    }
}

/* Hijos de una chamba: primero los parámetros y luego el cuerpo.
 * Hijos de un si: primero la condición y luego el cuerpo.
 * Hijos de 'ponele v a lista[pos]' y de 'lista[pos]': primero el valor y luego la posición. */
int nodo_num_hijos(const Nodo *nodo) {
    switch (nodo->clase) {
        case NODO_PROGRAMA: return nodo->num_sentencias;
        case NODO_CHAMBA: return nodo->num_elementos + nodo->num_sentencias;
        case NODO_LLAMADA:
        case NODO_CHORIZO: return nodo->num_elementos;
        case NODO_OPERACION: return 2;
        case NODO_SI: return 1 + nodo->num_sentencias;
        default: return (nodo->valor ? 1 : 0) + (nodo->posicion ? 1 : 0);
    }
}

Nodo *nodo_hijo(const Nodo *nodo, int i) {
    switch (nodo->clase) {
        case NODO_PROGRAMA: return nodo->sentencias[i];
        case NODO_CHAMBA:
            return i < nodo->num_elementos ? nodo->elementos[i] : nodo->sentencias[i - nodo->num_elementos];
        case NODO_LLAMADA:
        case NODO_CHORIZO: return nodo->elementos[i];
        case NODO_OPERACION: return i == 0 ? nodo->izq : nodo->der;
        case NODO_SI: return i == 0 ? nodo->valor : nodo->sentencias[i - 1];
        default: return i == 0 && nodo->valor ? nodo->valor : nodo->posicion;
    }
}
