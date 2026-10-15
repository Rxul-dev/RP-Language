/* Analizador sintáctico (parser) de descenso recursivo.
 *
 * Gramática (EBNF):
 *   programa    → { linea } FIN
 *   linea       → [ sentencia ] SALTO
 *   sentencia   → declaracion | asignacion | consulta | contame | devolveme | chamba | llamada
 *   declaracion → 'fijate' 'que' IDENT [ 'es' expresion ]
 *   asignacion  → 'ponele' expresion 'a' IDENT
 *   consulta    → 'que' 'onda' IDENT
 *   contame     → 'contame' expresion
 *   devolveme   → 'devolveme' [ expresion ]
 *   chamba      → 'chamba' IDENT '(' [ IDENT { ',' IDENT } ] ')' SALTO { linea } 'ya' 'estuvo'
 *   llamada     → IDENT '(' [ expresion { ',' expresion } ] ')'
 *   expresion   → termino { ('+' | '-') termino }
 *   termino     → factor { ('*' | '/') factor }
 *   factor      → CABAL | CON_VUELTO | CHISME | 'simon' | 'nel' | 'ni' 'maiz'
 *               | IDENT | llamada | '(' expresion ')'
 *
 * La precedencia sale de la gramática: '*' y '/' se agrupan antes que '+' y '-',
 * y todos asocian a la izquierda (10 - 2 - 3 = (10 - 2) - 3).
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
