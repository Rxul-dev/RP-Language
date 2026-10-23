/* ASIGNACIÓN:  ponele 20 a edad   ·   ponele 95 a notas[1]
 *
 * Cambia el valor de una variable que ya existe (o de una posición de un
 * chorizo). La variable adopta el tipo del nuevo valor (tipado dinámico).
 * Equivale a 'edad = 20' y 'notas[1] = 95' en Python.
 */
#include "instrucciones/instrucciones.h"

/* Sintaxis:  asignacion → 'ponele' expresion 'a' IDENT [ '[' expresion ']' ] */
Nodo *parsear_asignacion(Parser *p) {
    Nodo *n = instruccion_valor_a_nombre(p, NODO_ASIGNACION);
    if (n && verificar(p, TK_CORCHETE_ABRE)) {
        n->posicion = parsear_corchetes(p);
        if (!n->posicion) {
            ast_liberar(n);
            return NULL;
        }
    }
    return n;
}

/* Semántica: la variable debe poder cambiarse; con posición, debe ser un chorizo */
void verificar_asignacion(Semantico *sem, Nodo *nodo) {
    TipoRP tipo = tipo_expresion(sem, nodo->valor);
    Simbolo *s = variable_a_cambiar(sem, nodo->nombre, nodo->linea, "ponele");
    if (nodo->posicion) {
        verificar_posicion_de(sem, s ? s->tipo : RP_DESCONOCIDO, nodo->posicion, nodo->linea);
        if (s) enteros_agregar(&s->asignaciones, nodo->linea);
        return;  /* cambia un valor adentro: la variable sigue siendo un chorizo */
    }
    if (!s) return;
    anotar(nodo, tipo);
    s->tipo = tipo;  /* tipado dinámico: la variable adopta el tipo del nuevo valor */
    enteros_agregar(&s->asignaciones, nodo->linea);
    if (s->historial.datos[s->historial.cantidad - 1] != (int)tipo) enteros_agregar(&s->historial, (int)tipo);
}

/* Ejecución: reemplaza el valor guardado en la celda (o en la posición del chorizo) */
void ejecutar_asignacion(Interprete *in, const Nodo *nodo) {
    ValorRP v = evaluar(in, nodo->valor);  /* antes de liberar: 'ponele x a x' */
    ValorRP pos = nodo->posicion && !in->hay_error ? evaluar(in, nodo->posicion) : NADA;
    Celda *c = buscar_variable(in, nodo->nombre);
    if (!c) error_ejecucion(in, nodo->linea, NO_EXISTE, nodo->nombre);

    ValorRP *destino = NULL;
    if (!in->hay_error) destino = nodo->posicion ? chorizo_en(in, &c->valor, &pos, nodo->linea) : &c->valor;
    if (destino) {
        valor_liberar(destino);
        *destino = v;
    } else {
        valor_liberar(&v);
    }
    valor_liberar(&pos);
}
