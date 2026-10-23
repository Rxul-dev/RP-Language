/* Analizador semántico.
 *
 * Recorre el AST, llena la tabla de símbolos y verifica:
 *   1. Que una variable no se declare dos veces con 'fijate que'.
 *   2. Que toda variable usada o asignada con 'ponele' haya sido declarada antes.
 *   3. El tipo de cada valor (inferencia de tipos), anotándolo en el AST.
 *   4. Chambas: que no se repitan ni sus parámetros, que se llamen con la cantidad
 *      correcta de valores, que 'devolveme' solo aparezca dentro de una chamba y
 *      que una chamba no cambie variables globales.
 *   5. Operaciones: que '+ - * /' y las comparaciones se usen con tipos compatibles
 *      (si un tipo se sabe hasta ejecutar, como el de un parámetro, lo revisa el intérprete).
 *   6. Que la condición de un 'si' dé simon o nel.
 *   7. Chorizos: que 'metele', 'cuantos' y las posiciones se usen con un chorizo y que
 *      la posición sea un cabal.
 *
 * La verificación de cada instrucción está en su archivo de src/instrucciones/.
 */
#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "base/errores.h"
#include "fases/ast.h"
#include "fases/tabla_simbolos.h"

typedef struct {
    TablaSimbolos *tabla;
    ListaErrores *errores;
    const char *chamba_actual;  /* nombre de la chamba cuyo cuerpo se revisa, o NULL */
} Semantico;

void semantico_analizar(Semantico *sem, Nodo *programa);
void semantico_visitar(Semantico *sem, Nodo *sentencia);

/* ------------------------------------------------ para las verificaciones de cada instrucción */

/* Reporta un error semántico; siempre devuelve NULL */
void *error_semantico(Semantico *sem, int linea, const char *formato, ...);
/* Guarda en el nodo su tipo (AST decorado) */
void anotar(Nodo *nodo, TipoRP tipo);
/* 1 si el símbolo es del ámbito global */
int es_global(const Simbolo *s);
/* Busca una variable que se va a leer y registra el uso; NULL (con error) si no existe */
Simbolo *usar(Semantico *sem, const char *nombre, int linea);
/* La variable que va a cambiar 'ponele' o 'metele': debe existir, no ser una chamba y,
 * dentro de una chamba, no ser global. NULL (con error) si no se puede cambiar. */
Simbolo *variable_a_cambiar(Semantico *sem, const char *nombre, int linea, const char *instruccion);

#endif
