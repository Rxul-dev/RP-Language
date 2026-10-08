/* Analizador semántico.
 *
 * Recorre el AST, llena la tabla de símbolos y verifica:
 *   1. Que una variable no se declare dos veces con 'fijate que'.
 *   2. Que toda variable usada o asignada con 'ponele' haya sido declarada antes.
 *   3. El tipo de cada valor (inferencia de tipos), anotándolo en el AST.
 */
#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "ast.h"
#include "errores.h"
#include "tabla_simbolos.h"

typedef struct {
    TablaSimbolos *tabla;
    ListaErrores *errores;
} Semantico;

void semantico_analizar(Semantico *sem, Nodo *programa);
void semantico_visitar(Semantico *sem, Nodo *sentencia);

#endif
