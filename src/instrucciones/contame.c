/* CONTAME:  contame "Hola, " + nombre
 *
 * Muestra un valor en la pantalla. Equivale a 'print(...)' en Python.
 * Es la instrucción más sencilla: sirve de ejemplo para agregar otras.
 */
#include "instrucciones/instrucciones.h"

/* Sintaxis:  contame → 'contame' expresion */
Nodo *parsear_contame(Parser *p) {
    return instruccion_con_valor(p, NODO_CONTAME);
}

/* Semántica: solo hay que revisar el valor */
void verificar_contame(Semantico *sem, Nodo *nodo) {
    anotar(nodo, tipo_expresion(sem, nodo->valor));
}

/* Ejecución: calcula el valor y lo muestra */
void ejecutar_contame(Interprete *in, const Nodo *nodo) {
    ValorRP v = evaluar(in, nodo->valor);
    if (!in->hay_error) {
        char buf[512];
        valor_texto(&v, buf, sizeof buf);
        in->salida(in->contexto, v.tipo == RP_CHISME ? v.dato.chisme : buf);  /* un chisme largo no se corta */
    }
    valor_liberar(&v);
}
