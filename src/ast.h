/* Árbol de Sintaxis Abstracta (AST) de RP.
 *
 * Todos los nodos usan la misma estructura; el campo `clase` dice qué tipo de
 * nodo es y qué campos se usan. El campo `tipo` lo llena el analizador
 * semántico (AST decorado); en los literales lo fija el analizador sintáctico.
 */
#ifndef AST_H
#define AST_H

#include <stddef.h>

#include "valor.h"

typedef enum {
    NODO_PROGRAMA,      /* sentencias[]                                  */
    NODO_DECLARACION,   /* fijate que <nombre> [es <valor>]              */
    NODO_ASIGNACION,    /* ponele <valor> a <nombre>                     */
    NODO_CONSULTA_TIPO, /* que onda <nombre>                             */
    NODO_LITERAL,       /* 42, 3.5, "hola", simon, nel, ni maiz          */
    NODO_VARIABLE       /* nombre de variable usado como valor           */
} ClaseNodo;

typedef struct Nodo {
    ClaseNodo clase;
    int linea;

    char *nombre;        /* DECLARACION, ASIGNACION, CONSULTA_TIPO, VARIABLE */
    struct Nodo *valor;  /* DECLARACION (NULL si no tiene valor), ASIGNACION */

    char *lexema;        /* LITERAL: el texto tal como se escribió */
    ValorRP literal;     /* LITERAL: el valor ya convertido */

    int tiene_tipo;      /* 1 cuando `tipo` ya fue calculado */
    TipoRP tipo;

    struct Nodo **sentencias; /* PROGRAMA */
    int num_sentencias;
    int capacidad;
} Nodo;

Nodo *nodo_nuevo(ClaseNodo clase, int linea);
void nodo_agregar_sentencia(Nodo *programa, Nodo *sentencia);
void ast_liberar(Nodo *nodo);

/* Para recorrer y mostrar el árbol */
const char *nodo_etiqueta(const Nodo *nodo);
void nodo_atributos(const Nodo *nodo, char *buf, size_t n);
int nodo_num_hijos(const Nodo *nodo);
Nodo *nodo_hijo(const Nodo *nodo, int i);

#endif
