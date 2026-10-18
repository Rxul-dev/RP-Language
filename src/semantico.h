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
 */
#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "ast.h"
#include "errores.h"
#include "tabla_simbolos.h"

typedef struct {
    TablaSimbolos *tabla;
    ListaErrores *errores;
    const char *chamba_actual;  /* nombre de la chamba cuyo cuerpo se revisa, o NULL */
} Semantico;

void semantico_analizar(Semantico *sem, Nodo *programa);
void semantico_visitar(Semantico *sem, Nodo *sentencia);

#endif
