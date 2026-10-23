/* CHORIZOS (arreglos): una fila de valores, uno detrás de otro.
 *
 *     fijate que notas es [90, 85]     crea un chorizo           (notas = [90, 85])
 *     contame notas[0]                 lee una posición          (notas[0])
 *     ponele 95 a notas[1]             cambia una posición       (notas[1] = 95)   → asignacion.c
 *     metele 70 a notas                agrega al final           (notas.append(70))
 *     contame cuantos notas            cantidad de valores       (len(notas))
 *
 * Las posiciones empiezan en 0, como en Python.
 */
#include "instrucciones/instrucciones.h"

#include <stdio.h>

#include "base/util.h"

/* ============================================================ [90, 85] */

/* Sintaxis:  chorizo → '[' [ expresion { ',' expresion } ] ']' */
Nodo *parsear_chorizo(Parser *p) {
    Nodo *n = nodo_nuevo(NODO_CHORIZO, avanzar(p)->linea);
    if (leer_lista(p, n, TK_CORCHETE_CIERRA, parsear_expresion, "el chorizo")) return n;
    ast_liberar(n);
    return NULL;
}

/* Semántica: cada valor puede ser de cualquier tipo */
TipoRP tipo_chorizo(Semantico *sem, Nodo *nodo) {
    for (int i = 0; i < nodo->num_elementos; i++) tipo_expresion(sem, nodo->elementos[i]);
    anotar(nodo, RP_CHORIZO);
    return RP_CHORIZO;
}

ValorRP evaluar_chorizo(Interprete *in, const Nodo *nodo) {
    ValorRP chorizo = chorizo_nuevo();
    for (int i = 0; i < nodo->num_elementos && !in->hay_error; i++) chorizo_agregar(&chorizo, evaluar(in, nodo->elementos[i]));
    if (in->hay_error) valor_liberar(&chorizo);
    return chorizo;
}

/* ============================================================ notas[0] */

/* Sintaxis:  '[' expresion ']' */
Nodo *parsear_corchetes(Parser *p) {
    char buf[200];
    avanzar(p);  /* '[' */
    Nodo *posicion = parsear_expresion(p, "como posición dentro de '[ ]'");
    if (!posicion) return NULL;
    if (!verificar(p, TK_CORCHETE_CIERRA)) {
        error_en(p, actual(p), "falta ']' para cerrar la posición (se encontró %s)", describir(actual(p), buf, sizeof buf));
        ast_liberar(posicion);
        return NULL;
    }
    avanzar(p);
    return posicion;
}

Nodo *parsear_posicion(Parser *p, Nodo *chorizo) {
    int linea = actual(p)->linea;
    Nodo *posicion = parsear_corchetes(p);
    if (!posicion) {
        ast_liberar(chorizo);
        return NULL;
    }
    Nodo *n = nodo_nuevo(NODO_POSICION, linea);
    n->valor = chorizo;
    n->posicion = posicion;
    return n;
}

static void revisar_que_sea_chorizo(Semantico *sem, TipoRP tipo, int linea, const char *uso) {
    if (tipo != RP_DESCONOCIDO && tipo != RP_CHORIZO)
        error_semantico(sem, linea, "%s solo funciona con un chorizo, pero el valor es de tipo %s", uso,
                        tipo_rp_nombre(tipo));
}

void verificar_posicion_de(Semantico *sem, TipoRP tipo_chorizo, Nodo *posicion, int linea) {
    revisar_que_sea_chorizo(sem, tipo_chorizo, linea, "una posición '[ ]'");
    TipoRP tipo = tipo_expresion(sem, posicion);
    if (tipo != RP_DESCONOCIDO && tipo != RP_CABAL)
        error_semantico(sem, linea, "la posición dentro de '[ ]' debe ser un cabal (0, 1, 2...), pero es de tipo %s",
                        tipo_rp_nombre(tipo));
}

/* Semántica: los valores de un chorizo pueden ser de cualquier tipo: el resultado se sabe al ejecutar */
TipoRP tipo_posicion(Semantico *sem, Nodo *nodo) {
    verificar_posicion_de(sem, tipo_expresion(sem, nodo->valor), nodo->posicion, nodo->linea);
    anotar(nodo, RP_DESCONOCIDO);
    return RP_DESCONOCIDO;
}

ValorRP *chorizo_en(Interprete *in, ValorRP *chorizo, const ValorRP *pos, int linea) {
    if (chorizo->tipo != RP_CHORIZO) {
        error_ejecucion(in, linea, "solo un chorizo tiene posiciones '[ ]', pero el valor es de tipo %s",
                        tipo_rp_nombre(chorizo->tipo));
        return NULL;
    }
    if (pos->tipo != RP_CABAL) {
        error_ejecucion(in, linea, "la posición dentro de '[ ]' debe ser un cabal, pero es de tipo %s",
                        tipo_rp_nombre(pos->tipo));
        return NULL;
    }
    int cantidad = chorizo->dato.chorizo->cantidad;
    if (pos->dato.cabal < 0 || pos->dato.cabal >= cantidad) {
        if (cantidad == 0)
            error_ejecucion(in, linea, "la posición %lld no existe: el chorizo está vacío", pos->dato.cabal);
        else
            error_ejecucion(in, linea, "la posición %lld no existe: el chorizo tiene %d valor(es), en las posiciones "
                            "0 a %d", pos->dato.cabal, cantidad, cantidad - 1);
        return NULL;
    }
    return &chorizo->dato.chorizo->valores[pos->dato.cabal];
}

ValorRP evaluar_posicion(Interprete *in, const Nodo *nodo) {
    ValorRP chorizo = evaluar(in, nodo->valor);
    ValorRP pos = in->hay_error ? NADA : evaluar(in, nodo->posicion);
    ValorRP resultado = NADA;
    if (!in->hay_error) {
        ValorRP *valor = chorizo_en(in, &chorizo, &pos, nodo->linea);
        if (valor) resultado = valor_copiar(valor);
    }
    valor_liberar(&chorizo);
    valor_liberar(&pos);
    return resultado;
}

/* ============================================================ cuantos notas */

/* Sintaxis:  'cuantos' factor   (en 'cuantos notas + 1' primero se cuenta y luego se suma) */
Nodo *parsear_cuantos(Parser *p) {
    Nodo *n = nodo_nuevo(NODO_CUANTOS, avanzar(p)->linea);
    n->valor = parsear_factor(p, "después de 'cuantos'");
    if (n->valor) return n;
    ast_liberar(n);
    return NULL;
}

/* Semántica: cuenta los valores de un chorizo o las letras de un chisme; siempre da un cabal */
TipoRP tipo_cuantos(Semantico *sem, Nodo *nodo) {
    TipoRP tipo = tipo_expresion(sem, nodo->valor);
    if (tipo != RP_DESCONOCIDO && tipo != RP_CHORIZO && tipo != RP_CHISME)
        error_semantico(sem, nodo->linea, "'cuantos' cuenta los valores de un chorizo o las letras de un chisme, "
                        "pero el valor es de tipo %s", tipo_rp_nombre(tipo));
    anotar(nodo, RP_CABAL);
    return RP_CABAL;
}

ValorRP evaluar_cuantos(Interprete *in, const Nodo *nodo) {
    ValorRP v = evaluar(in, nodo->valor);
    ValorRP resultado = {RP_CABAL, {0}};
    if (v.tipo == RP_CHORIZO)
        resultado.dato.cabal = v.dato.chorizo->cantidad;
    else if (v.tipo == RP_CHISME)
        resultado.dato.cabal = (long long)utf8_ancho(v.dato.chisme);
    else if (!in->hay_error)
        error_ejecucion(in, nodo->linea, "'cuantos' cuenta los valores de un chorizo o las letras de un chisme, "
                        "pero el valor es de tipo %s", tipo_rp_nombre(v.tipo));
    valor_liberar(&v);
    return resultado;
}

/* ============================================================ metele 70 a notas */

/* Sintaxis:  metele → 'metele' expresion 'a' IDENT */
Nodo *parsear_metele(Parser *p) {
    return instruccion_valor_a_nombre(p, NODO_METELE);
}

/* Semántica: igual que 'ponele', pero la variable debe ser un chorizo */
void verificar_metele(Semantico *sem, Nodo *nodo) {
    tipo_expresion(sem, nodo->valor);
    Simbolo *s = variable_a_cambiar(sem, nodo->nombre, nodo->linea, "metele");
    if (!s) return;
    revisar_que_sea_chorizo(sem, s->tipo, nodo->linea, "'metele'");
    enteros_agregar(&s->asignaciones, nodo->linea);
    anotar(nodo, RP_CHORIZO);
}

/* Ejecución: agrega una copia del valor al final del chorizo */
void ejecutar_metele(Interprete *in, const Nodo *nodo) {
    ValorRP v = evaluar(in, nodo->valor);
    Celda *c = in->hay_error ? NULL : buscar_variable(in, nodo->nombre);
    if (!in->hay_error && !c)
        error_ejecucion(in, nodo->linea, NO_EXISTE, nodo->nombre);
    else if (!in->hay_error && c->valor.tipo != RP_CHORIZO)
        error_ejecucion(in, nodo->linea, "'metele' solo funciona con un chorizo, pero '%s' es de tipo %s",
                        nodo->nombre, tipo_rp_nombre(c->valor.tipo));
    if (in->hay_error)
        valor_liberar(&v);
    else
        chorizo_agregar(&c->valor, v);
}
