/* ANALIZADOR SEMÁNTICO (fase 3)
 *
 * ¿Qué hace?
 *   Revisa que el programa tenga sentido, aunque esté bien escrito.
 *   'ponele 5 a sueldo' está bien escrito, pero no tiene sentido si 'sueldo'
 *   nunca se creó con 'fijate que'.
 *
 *   Mientras revisa, llena la tabla de símbolos (todas las variables y chambas)
 *   y anota en el AST el tipo de cada valor.
 *
 * ¿Cómo está ordenado?
 *   La verificación de cada instrucción está en su archivo de src/instrucciones/
 *   (verificar_declaracion() en declaracion.c, verificar_si() en si.c...).
 *   semantico_visitar() elige cuál usar; aquí queda lo que todas comparten.
 */
#include "fases/semantico.h"

#include <stdarg.h>
#include <stddef.h>

#include "instrucciones/instrucciones.h"

void *error_semantico(Semantico *sem, int linea, const char *formato, ...) {
    va_list args;
    va_start(args, formato);
    errores_agregarv(sem->errores, FASE_SEMANTICA, linea, 0, formato, args);
    va_end(args);
    return NULL;
}

void anotar(Nodo *nodo, TipoRP tipo) {
    nodo->tipo = tipo;
    nodo->tiene_tipo = 1;
}

int es_global(const Simbolo *s) {
    return s->ambito[0] == 'g';  /* "global"; los demás ámbitos empiezan con "chamba " */
}

Simbolo *usar(Semantico *sem, const char *nombre, int linea) {
    Simbolo *s = ts_buscar(sem->tabla, nombre);
    if (!s)
        return error_semantico(sem, linea, "la variable '%s' no ha sido declarada; declárela antes con 'fijate que %s'",
                               nombre, nombre);
    if (s->clase == SIM_CHAMBA)
        return error_semantico(sem, linea, "'%s' es una chamba, no una variable; para usarla llámela con '%s(...)'",
                               nombre, nombre);
    enteros_agregar(&s->usos, linea);
    return s;
}

Simbolo *variable_a_cambiar(Semantico *sem, const char *nombre, int linea, const char *instruccion) {
    Simbolo *s = ts_buscar(sem->tabla, nombre);
    if (!s)
        return error_semantico(sem, linea, "no se puede cambiar '%s' porque no ha sido declarada; declárela primero "
                               "con 'fijate que %s'", nombre, nombre);
    if (s->clase == SIM_CHAMBA)
        return error_semantico(sem, linea, "'%s' es una chamba; con '%s' solo se cambia una variable", nombre,
                               instruccion);
    /* Una chamba lee las variables globales pero no las cambia: así su tipo en la tabla sigue siendo
     * cierto y el resultado de la chamba sale solo por 'devolveme' */
    if (sem->chamba_actual && es_global(s))
        return error_semantico(sem, linea, "la chamba '%s' no puede cambiar la variable global '%s'; devuelva el "
                               "valor con 'devolveme' y asígnelo afuera", sem->chamba_actual, nombre);
    return s;
}

void semantico_visitar(Semantico *sem, Nodo *sentencia) {
    switch (sentencia->clase) {
        case NODO_DECLARACION: verificar_declaracion(sem, sentencia); break;
        case NODO_ASIGNACION: verificar_asignacion(sem, sentencia); break;
        case NODO_CONSULTA_TIPO: verificar_consulta(sem, sentencia); break;
        case NODO_CONTAME: verificar_contame(sem, sentencia); break;
        case NODO_DEVOLVEME: verificar_devolveme(sem, sentencia); break;
        case NODO_SI: verificar_si(sem, sentencia); break;
        case NODO_CHAMBA: verificar_chamba(sem, sentencia); break;
        case NODO_METELE: verificar_metele(sem, sentencia); break;
        case NODO_LLAMADA: tipo_llamada(sem, sentencia); break;
        default: break;
    }
}

void semantico_analizar(Semantico *sem, Nodo *programa) {
    for (int i = 0; i < programa->num_sentencias; i++) semantico_visitar(sem, programa->sentencias[i]);
}
