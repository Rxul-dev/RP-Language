#include "interprete.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void salida_pantalla(void *contexto, const char *texto) {
    (void)contexto;
    printf("%s\n", texto);
}

void interprete_iniciar(Interprete *in, FuncionSalida salida, void *contexto) {
    memset(in, 0, sizeof *in);
    in->salida = salida ? salida : salida_pantalla;
    in->contexto = contexto;
}

static Celda *buscar(Interprete *in, const char *nombre) {
    for (int i = 0; i < in->cantidad; i++) {
        if (strcmp(in->memoria[i].nombre, nombre) == 0) return &in->memoria[i];
    }
    return NULL;
}

static Celda *nueva_celda(Interprete *in, const char *nombre) {
    if (in->cantidad == in->capacidad) {
        in->capacidad = in->capacidad ? in->capacidad * 2 : 8;
        in->memoria = xrealloc(in->memoria, (size_t)in->capacidad * sizeof *in->memoria);
    }
    Celda *c = &in->memoria[in->cantidad++];
    c->nombre = xstrdup(nombre);
    c->valor.tipo = RP_NI_MAIZ;
    return c;
}

/* Devuelve una copia del valor (el chisme se duplica para que cada variable tenga el suyo) */
static ValorRP evaluar(Interprete *in, const Nodo *valor) {
    if (valor->clase == NODO_LITERAL) return valor_copiar(&valor->literal);
    return valor_copiar(&buscar(in, valor->nombre)->valor);
}

void interprete_ejecutar_sentencia(Interprete *in, const Nodo *s) {
    switch (s->clase) {
        case NODO_DECLARACION: {
            ValorRP v = {RP_NI_MAIZ, {0}};
            if (s->valor) v = evaluar(in, s->valor);
            nueva_celda(in, s->nombre)->valor = v;
            break;
        }
        case NODO_ASIGNACION: {
            ValorRP v = evaluar(in, s->valor);  /* antes de liberar: 'ponele x a x' */
            Celda *c = buscar(in, s->nombre);
            valor_liberar(&c->valor);
            c->valor = v;
            break;
        }
        case NODO_CONSULTA_TIPO:
            /* El tipo viaja con el valor (unión etiquetada), igual que type() en Python */
            in->salida(in->contexto, tipo_rp_nombre(buscar(in, s->nombre)->valor.tipo));
            break;
        default:
            break;
    }
}

void interprete_ejecutar(Interprete *in, const Nodo *programa) {
    for (int i = 0; i < programa->num_sentencias; i++) interprete_ejecutar_sentencia(in, programa->sentencias[i]);
}

void interprete_liberar(Interprete *in) {
    for (int i = 0; i < in->cantidad; i++) {
        free(in->memoria[i].nombre);
        valor_liberar(&in->memoria[i].valor);
    }
    free(in->memoria);
    in->memoria = NULL;
    in->cantidad = in->capacidad = 0;
}
