/* INTÉRPRETE (fase 4)
 *
 * ¿Qué hace?
 *   Ejecuta el programa: recorre el AST instrucción por instrucción y hace lo
 *   que cada una dice (crear variables, mostrar valores, llamar chambas...).
 *
 * ¿Cómo guarda las variables?
 *   En una lista llamada "memoria". Cada variable es una celda con su nombre y
 *   su valor. Cuando se llama una chamba, sus parámetros y variables se agregan
 *   al final de la lista y se borran cuando la chamba termina.
 *
 * ¿Cómo está ordenado?
 *   Lo que hace cada instrucción está en su archivo de src/instrucciones/
 *   (ejecutar_contame() en contame.c, evaluar() en expresiones.c...).
 *   Aquí queda la memoria y interprete_ejecutar_sentencia(), que elige qué ejecutar.
 */
#include "fases/interprete.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/util.h"
#include "instrucciones/instrucciones.h"

static void salida_pantalla(void *contexto, const char *texto) {
    (void)contexto;
    printf("%s\n", texto);
}

void interprete_iniciar(Interprete *in, FuncionSalida salida, void *contexto) {
    memset(in, 0, sizeof *in);
    in->salida = salida ? salida : salida_pantalla;
    in->contexto = contexto;
    in->devuelto = NADA;
}

void error_ejecucion(Interprete *in, int linea, const char *formato, ...) {
    if (in->hay_error) return;
    in->hay_error = 1;
    memset(&in->error, 0, sizeof in->error);
    in->error.fase = FASE_EJECUCION;
    in->error.linea = linea;
    va_list args;
    va_start(args, formato);
    vsnprintf(in->error.mensaje, sizeof in->error.mensaje, formato, args);
    va_end(args);
}

/* ------------------------------------------------------------ memoria */

Celda *buscar_variable(Interprete *in, const char *nombre) {
    for (int i = in->cantidad - 1; i >= in->marco; i--) {
        if (strcmp(in->memoria[i].nombre, nombre) == 0) return &in->memoria[i];
    }
    if (in->profundidad > 0) {
        for (int i = in->globales - 1; i >= 0; i--) {
            if (strcmp(in->memoria[i].nombre, nombre) == 0) return &in->memoria[i];
        }
    }
    return NULL;
}

Celda *nueva_celda(Interprete *in, const char *nombre) {
    in->memoria = crecer(in->memoria, in->cantidad, &in->capacidad, sizeof *in->memoria);
    Celda *c = &in->memoria[in->cantidad++];
    c->nombre = xstrdup(nombre);
    c->valor = NADA;
    return c;
}

void liberar_hasta(Interprete *in, int inicio) {
    while (in->cantidad > inicio) {
        Celda *c = &in->memoria[--in->cantidad];
        free(c->nombre);
        valor_liberar(&c->valor);
    }
}

/* ------------------------------------------------------------ ejecución */

void ejecutar_cuerpo(Interprete *in, const Nodo *bloque) {
    for (int i = 0; i < bloque->num_sentencias && !in->devolviendo && !in->hay_error; i++)
        interprete_ejecutar_sentencia(in, bloque->sentencias[i]);
}

int interprete_ejecutar_sentencia(Interprete *in, const Nodo *s) {
    switch (s->clase) {
        case NODO_DECLARACION: ejecutar_declaracion(in, s); break;
        case NODO_ASIGNACION: ejecutar_asignacion(in, s); break;
        case NODO_CONSULTA_TIPO: ejecutar_consulta(in, s); break;
        case NODO_CONTAME: ejecutar_contame(in, s); break;
        case NODO_DEVOLVEME: ejecutar_devolveme(in, s); break;
        case NODO_SI: ejecutar_si(in, s); break;
        case NODO_CHAMBA: ejecutar_chamba(in, s); break;
        case NODO_METELE: ejecutar_metele(in, s); break;
        case NODO_LLAMADA: {
            ValorRP v = evaluar_llamada(in, s);  /* como sentencia sola, lo que devuelve se descarta */
            valor_liberar(&v);
            break;
        }
        default:
            break;
    }
    return !in->hay_error;
}

int interprete_ejecutar(Interprete *in, const Nodo *programa) {
    for (int i = 0; i < programa->num_sentencias; i++) {
        if (!interprete_ejecutar_sentencia(in, programa->sentencias[i])) return 0;
    }
    return 1;
}

void interprete_liberar(Interprete *in) {
    liberar_hasta(in, 0);
    free(in->memoria);
    free(in->chambas);
    valor_liberar(&in->devuelto);
    in->memoria = NULL;
    in->chambas = NULL;
    in->cantidad = in->capacidad = in->num_chambas = in->capacidad_chambas = 0;
}
