/* Árbol de Sintaxis Abstracta (AST) de RP.
 *
 * Todos los nodos usan la misma estructura; el campo `clase` dice qué tipo de
 * nodo es y qué campos se usan. El campo `tipo` lo llena el analizador
 * semántico (AST decorado); en los literales lo fija el analizador sintáctico.
 */
#ifndef AST_H
#define AST_H

#include <stddef.h>

#include "base/valor.h"

typedef enum {
    NODO_PROGRAMA,      /* sentencias[]                                    */
    NODO_DECLARACION,   /* fijate que <nombre> [es <valor>]                */
    NODO_ASIGNACION,    /* ponele <valor> a <nombre>  o  a <nombre>[<pos>] */
    NODO_CONSULTA_TIPO, /* que onda <nombre>                               */
    NODO_CONTAME,       /* contame <valor>                                 */
    NODO_SI,            /* si <condición> ... ya estuvo                    */
    NODO_CHAMBA,        /* chamba <nombre>(<parámetros>) ... ya estuvo     */
    NODO_PARAMETRO,     /* un parámetro de una chamba                      */
    NODO_DEVOLVEME,     /* devolveme [<valor>]                             */
    NODO_METELE,        /* metele <valor> a <nombre>                       */
    /* Expresiones (dan un valor) */
    NODO_LITERAL,       /* 42, 3.5, "hola", simon, nel, ni maiz            */
    NODO_VARIABLE,      /* nombre de variable usado como valor             */
    NODO_OPERACION,     /* <izq> + <der>   (también - * / == != < > <= >=)   */
    NODO_LLAMADA,       /* <nombre>(<argumentos>)                          */
    NODO_CHORIZO,       /* [<valor>, <valor>, ...]                         */
    NODO_POSICION,      /* <chorizo>[<pos>]                                */
    NODO_CUANTOS        /* cuantos <valor>                                 */
} ClaseNodo;

typedef struct Nodo {
    ClaseNodo clase;
    int linea;

    char *nombre;        /* DECLARACION, ASIGNACION, CONSULTA_TIPO, VARIABLE, CHAMBA, PARAMETRO,
                            LLAMADA, METELE */
    struct Nodo *valor;  /* el valor de DECLARACION y DEVOLVEME (NULL si no tienen), ASIGNACION,
                            CONTAME, METELE y CUANTOS; SI: la condición; POSICION: el chorizo */
    struct Nodo *posicion; /* POSICION: la posición; ASIGNACION: la de 'a lista[pos]' (o NULL) */

    char operador[3];    /* OPERACION: "+", "-", "*", "/", "==", "!=", "<", ">", "<=" o ">=" */
    struct Nodo *izq;    /* OPERACION */
    struct Nodo *der;    /* OPERACION */

    struct Nodo **elementos; /* CHAMBA: los nodos PARAMETRO; LLAMADA: los argumentos; CHORIZO: sus valores */
    int num_elementos;
    int capacidad_elementos;

    char *lexema;        /* LITERAL: el texto tal como se escribió */
    ValorRP literal;     /* LITERAL: el valor ya convertido */

    int tiene_tipo;      /* 1 cuando `tipo` ya fue calculado */
    TipoRP tipo;

    struct Nodo **sentencias; /* PROGRAMA, CHAMBA y SI (el cuerpo) */
    int num_sentencias;
    int capacidad;
} Nodo;

Nodo *nodo_nuevo(ClaseNodo clase, int linea);
Nodo *nodo_con_nombre(ClaseNodo clase, int linea, const char *nombre);
void nodo_agregar_sentencia(Nodo *programa, Nodo *sentencia);
void nodo_agregar_elemento(Nodo *nodo, Nodo *elemento);
void ast_liberar(Nodo *nodo);

/* Para recorrer y mostrar el árbol */
const char *nodo_etiqueta(const Nodo *nodo);
void nodo_atributos(const Nodo *nodo, char *buf, size_t n);
int nodo_num_hijos(const Nodo *nodo);
Nodo *nodo_hijo(const Nodo *nodo, int i);

#endif
