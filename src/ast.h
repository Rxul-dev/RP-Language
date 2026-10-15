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
    NODO_VARIABLE,      /* nombre de variable usado como valor           */
    NODO_OPERACION,     /* <izq> + <der>   (también - * /)               */
    NODO_CHAMBA,        /* chamba <nombre>(<parámetros>) ... ya estuvo   */
    NODO_PARAMETRO,     /* un parámetro de una chamba                    */
    NODO_LLAMADA,       /* <nombre>(<argumentos>)                        */
    NODO_DEVOLVEME,     /* devolveme [<valor>]                           */
    NODO_CONTAME        /* contame <valor>                               */
} ClaseNodo;

typedef struct Nodo {
    ClaseNodo clase;
    int linea;

    char *nombre;        /* DECLARACION, ASIGNACION, CONSULTA_TIPO, VARIABLE, CHAMBA, PARAMETRO, LLAMADA */
    struct Nodo *valor;  /* DECLARACION y DEVOLVEME (NULL si no tiene valor), ASIGNACION, CONTAME */

    char operador;       /* OPERACION: '+', '-', '*' o '/' */
    struct Nodo *izq;    /* OPERACION */
    struct Nodo *der;    /* OPERACION */

    struct Nodo **parametros; /* CHAMBA: nodos PARAMETRO; LLAMADA: los argumentos */
    int num_parametros;
    int capacidad_parametros;

    char *lexema;        /* LITERAL: el texto tal como se escribió */
    ValorRP literal;     /* LITERAL: el valor ya convertido */

    int tiene_tipo;      /* 1 cuando `tipo` ya fue calculado */
    TipoRP tipo;

    struct Nodo **sentencias; /* PROGRAMA y CHAMBA (el cuerpo) */
    int num_sentencias;
    int capacidad;
} Nodo;

Nodo *nodo_nuevo(ClaseNodo clase, int linea);
void nodo_agregar_sentencia(Nodo *programa, Nodo *sentencia);
void nodo_agregar_parametro(Nodo *nodo, Nodo *parametro);
void ast_liberar(Nodo *nodo);

/* Para recorrer y mostrar el árbol */
const char *nodo_etiqueta(const Nodo *nodo);
void nodo_atributos(const Nodo *nodo, char *buf, size_t n);
int nodo_num_hijos(const Nodo *nodo);
Nodo *nodo_hijo(const Nodo *nodo, int i);

#endif
