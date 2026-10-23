/* CONSULTA DE TIPO:  que onda edad
 *
 * Muestra el tipo que tiene una variable en ese momento.
 * Equivale a 'print(type(edad))' en Python.
 */
#include "instrucciones/instrucciones.h"

/* Sintaxis:  consulta → 'que' 'onda' IDENT */
Nodo *parsear_consulta(Parser *p) {
    const Token *inicio = avanzar(p);
    char buf[200];
    if (!verificar(p, TK_ONDA))
        return error_en(p, actual(p),
                        "se esperaba 'onda' después de 'que'; para ver el tipo use 'que onda <nombre>' "
                        "(se encontró %s)", describir(actual(p), buf, sizeof buf));
    avanzar(p);

    const Token *nombre = leer_nombre(p, "variable", "después de 'que onda'");
    return nombre ? nodo_con_nombre(NODO_CONSULTA_TIPO, inicio->linea, nombre->lexema) : NULL;
}

/* Semántica: la variable debe existir */
void verificar_consulta(Semantico *sem, Nodo *nodo) {
    Simbolo *s = usar(sem, nodo->nombre, nodo->linea);
    if (s) anotar(nodo, sem->chamba_actual && es_global(s) ? RP_DESCONOCIDO : s->tipo);
}

/* Ejecución: el tipo viaja con el valor (unión etiquetada), igual que type() en Python */
void ejecutar_consulta(Interprete *in, const Nodo *nodo) {
    Celda *c = buscar_variable(in, nodo->nombre);
    if (c)
        in->salida(in->contexto, tipo_rp_nombre(c->valor.tipo));
    else
        error_ejecucion(in, nodo->linea, NO_EXISTE, nodo->nombre);
}
