/* Las instrucciones de RP, cada una en su propio archivo.
 *
 * Cada archivo tiene las tres partes de su instrucción, una por fase:
 *   parsear_X()    fase 2 · sintáctico: revisa cómo se escribe y arma su nodo del AST
 *   verificar_X()  fase 3 · semántico:  revisa que tenga sentido y anota los tipos
 *   ejecutar_X()   fase 4 · intérprete: hace lo que dice la instrucción
 * Las expresiones (lo que da un valor) usan tipo_X() y evaluar_X() en lugar de
 * verificar_X() y ejecutar_X().
 *
 * Para agregar una instrucción nueva: cree su archivo con las tres funciones,
 * declárelas aquí, agréguele un nodo en fases/ast.h y llámelas desde
 * sentencia() en sintactico.c, semantico_visitar() en semantico.c e
 * interprete_ejecutar_sentencia() en interprete.c. contame.c es el ejemplo más
 * sencillo.
 */
#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include "fases/interprete.h"
#include "fases/semantico.h"
#include "fases/sintactico.h"

/* expresiones.c · valores y operaciones:  2 + x * 3,  edad >= 18 */
Nodo *parsear_expresion(Parser *p, const char *contexto);
Nodo *parsear_factor(Parser *p, const char *contexto);  /* un solo valor: 5, x, notas[0], (1 + 2) */
TipoRP tipo_expresion(Semantico *sem, Nodo *valor);
ValorRP evaluar(Interprete *in, const Nodo *valor);  /* devuelve un valor nuevo, que hay que liberar */

/* declaracion.c · fijate que edad es 19 */
Nodo *parsear_declaracion(Parser *p);
void verificar_declaracion(Semantico *sem, Nodo *nodo);
void ejecutar_declaracion(Interprete *in, const Nodo *nodo);

/* asignacion.c · ponele 20 a edad */
Nodo *parsear_asignacion(Parser *p);
void verificar_asignacion(Semantico *sem, Nodo *nodo);
void ejecutar_asignacion(Interprete *in, const Nodo *nodo);

/* consulta.c · que onda edad */
Nodo *parsear_consulta(Parser *p);
void verificar_consulta(Semantico *sem, Nodo *nodo);
void ejecutar_consulta(Interprete *in, const Nodo *nodo);

/* contame.c · contame edad + 1 */
Nodo *parsear_contame(Parser *p);
void verificar_contame(Semantico *sem, Nodo *nodo);
void ejecutar_contame(Interprete *in, const Nodo *nodo);

/* si.c · si edad >= 18 ... ya estuvo */
Nodo *parsear_si(Parser *p);
void verificar_si(Semantico *sem, Nodo *nodo);
void ejecutar_si(Interprete *in, const Nodo *nodo);

/* chambas.c · chamba sumar(x, y) ... ya estuvo,  devolveme x + y,  sumar(1, 2) */
Nodo *parsear_chamba(Parser *p);
void verificar_chamba(Semantico *sem, Nodo *nodo);
void ejecutar_chamba(Interprete *in, const Nodo *nodo);
Nodo *parsear_devolveme(Parser *p);
void verificar_devolveme(Semantico *sem, Nodo *nodo);
void ejecutar_devolveme(Interprete *in, const Nodo *nodo);
Nodo *parsear_llamada(Parser *p);
TipoRP tipo_llamada(Semantico *sem, Nodo *nodo);
ValorRP evaluar_llamada(Interprete *in, const Nodo *nodo);

/* chorizos.c · [90, 85],  notas[0],  metele 70 a notas,  cuantos notas */
Nodo *parsear_chorizo(Parser *p);
TipoRP tipo_chorizo(Semantico *sem, Nodo *nodo);
ValorRP evaluar_chorizo(Interprete *in, const Nodo *nodo);
Nodo *parsear_corchetes(Parser *p);               /* '[' expresion ']': devuelve la posición */
Nodo *parsear_posicion(Parser *p, Nodo *chorizo);  /* lee '[pos]' después de un valor */
TipoRP tipo_posicion(Semantico *sem, Nodo *nodo);
ValorRP evaluar_posicion(Interprete *in, const Nodo *nodo);
/* Revisa que `tipo_chorizo` sea un chorizo y que la posición sea un cabal */
void verificar_posicion_de(Semantico *sem, TipoRP tipo_chorizo, Nodo *posicion, int linea);
Nodo *parsear_cuantos(Parser *p);
TipoRP tipo_cuantos(Semantico *sem, Nodo *nodo);
ValorRP evaluar_cuantos(Interprete *in, const Nodo *nodo);
Nodo *parsear_metele(Parser *p);
void verificar_metele(Semantico *sem, Nodo *nodo);
void ejecutar_metele(Interprete *in, const Nodo *nodo);
/* Revisa al ejecutar que `pos` sea una posición válida de `chorizo`; NULL (con error) si no */
ValorRP *chorizo_en(Interprete *in, ValorRP *chorizo, const ValorRP *pos, int linea);

#endif
