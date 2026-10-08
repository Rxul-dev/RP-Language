/* Analizador sintáctico (parser) de descenso recursivo.
 *
 * Gramática (EBNF):
 *   programa    → { linea } FIN
 *   linea       → [ sentencia ] SALTO
 *   sentencia   → declaracion | asignacion | consulta
 *   declaracion → 'fijate' 'que' IDENT [ 'es' valor ]
 *   asignacion  → 'ponele' valor 'a' IDENT
 *   consulta    → 'que' 'onda' IDENT
 *   valor       → CABAL | CON_VUELTO | CHISME | 'simon' | 'nel' | 'ni' 'maiz' | IDENT
 *
 * Cada regla de la gramática es una función. Recuperación de errores en modo
 * pánico: se reporta el error, se descarta el resto de la línea y se sigue.
 */
#ifndef SINTACTICO_H
#define SINTACTICO_H

#include "ast.h"
#include "errores.h"
#include "token.h"

Nodo *sintactico_analizar(const ListaTokens *tokens, ListaErrores *errores);

#endif
