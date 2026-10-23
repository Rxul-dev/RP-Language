/* Analizador sintáctico (parser) de descenso recursivo.
 *
 * Gramática (EBNF):
 *   programa    → { linea } FIN
 *   linea       → [ sentencia ] SALTO
 *   sentencia   → declaracion | asignacion | consulta | contame | devolveme | si | chamba
 *               | metele | llamada
 *   declaracion → 'fijate' 'que' IDENT [ 'es' expresion ]
 *   asignacion  → 'ponele' expresion 'a' IDENT [ '[' expresion ']' ]
 *   consulta    → 'que' 'onda' IDENT
 *   contame     → 'contame' expresion
 *   devolveme   → 'devolveme' [ expresion ]
 *   si          → 'si' expresion SALTO bloque
 *   chamba      → 'chamba' IDENT '(' [ IDENT { ',' IDENT } ] ')' SALTO bloque
 *   metele      → 'metele' expresion 'a' IDENT
 *   bloque      → { linea } 'ya' 'estuvo'
 *   llamada     → IDENT '(' [ expresion { ',' expresion } ] ')'
 *   expresion   → suma [ ('==' | '!=' | '<' | '>' | '<=' | '>=') suma ]
 *   suma        → termino { ('+' | '-') termino }
 *   termino     → factor { ('*' | '/') factor }
 *   factor      → primario { '[' expresion ']' }
 *   primario    → CABAL | CON_VUELTO | CHISME | 'simon' | 'nel' | 'ni' 'maiz'
 *               | IDENT | llamada | chorizo | 'cuantos' factor | '(' expresion ')'
 *   chorizo     → '[' [ expresion { ',' expresion } ] ']'
 *
 * La precedencia sale de la gramática: primero '*' y '/', luego '+' y '-', y al
 * final la comparación. '+ - * /' asocian a la izquierda (10 - 2 - 3 = (10 - 2) - 3).
 *
 * Cada regla de la gramática es una función que está en el archivo de su
 * instrucción (src/instrucciones/). Aquí está lo que todas comparten.
 * Recuperación de errores en modo pánico: se reporta el error, se descarta el
 * resto de la línea y se sigue.
 */
#ifndef SINTACTICO_H
#define SINTACTICO_H

#include <stddef.h>

#include "base/errores.h"
#include "fases/ast.h"
#include "fases/token.h"

Nodo *sintactico_analizar(const ListaTokens *tokens, ListaErrores *errores);

/* ------------------------------------------------ para las reglas de cada instrucción */

typedef struct {
    const ListaTokens *tokens;
    int pos;
    ListaErrores *errores;
    const Token *chamba_abierta;  /* el nombre de la chamba cuyo cuerpo se está leyendo, o NULL */
    int si_abiertos;              /* cuántos 'si' se están leyendo, uno dentro de otro */
} Parser;

const Token *actual(Parser *p);
const Token *siguiente(Parser *p);
const Token *avanzar(Parser *p);              /* devuelve el token en que estaba */
int verificar(Parser *p, TipoToken tipo);     /* 1 si el token actual es de ese tipo */

/* Texto para los mensajes: 'edad', el fin de la línea, ... */
const char *describir(const Token *t, char *buf, size_t n);
/* Reporta un error sintáctico en el token `t`; siempre devuelve NULL */
void *error_en(Parser *p, const Token *t, const char *formato, ...);

/* `de_que` es "variable", "chamba" o "parámetro"; NULL si no hay un nombre válido */
const Token *leer_nombre(Parser *p, const char *de_que, const char *contexto);

/* Lee '[ elemento { ',' elemento } ] <cierre>' y agrega cada elemento a `destino`.
 * `leer_uno` lee un elemento (una expresión, un parámetro...); `que` describe la lista
 * para los mensajes ("la llamada a 'sumar'"). Devuelve 0 si hubo error. */
typedef Nodo *(*LeerElemento)(Parser *p, const char *contexto);
int leer_lista(Parser *p, Nodo *destino, TipoToken cierre, LeerElemento leer_uno, const char *que);

/* '<palabra> expresion'  →  nodo `clase` con ese valor (contame, devolveme) */
Nodo *instruccion_con_valor(Parser *p, ClaseNodo clase);
/* '<palabra> expresion 'a' IDENT'  →  nodo `clase` con valor y nombre (ponele, metele) */
Nodo *instruccion_valor_a_nombre(Parser *p, ClaseNodo clase);

/* bloque → { linea } 'ya' 'estuvo'. Agrega las sentencias a `destino`; 0 si hubo error */
int bloque(Parser *p, Nodo *destino, const char *que);
/* Modo pánico para un bloque con el encabezado mal escrito: descarta hasta su 'ya estuvo' */
void saltar_bloque(Parser *p);

#endif
