/* DECLARACIÓN:  fijate que edad es 19
 *
 * Crea una variable. Sin 'es <valor>' la variable empieza en ni maiz.
 * Equivale a 'edad = 19' en Python.
 */
#include "instrucciones/instrucciones.h"

/* Sintaxis:  declaracion → 'fijate' 'que' IDENT [ 'es' expresion ] */
Nodo *parsear_declaracion(Parser *p) {
    const Token *inicio = avanzar(p);
    char buf[200];
    if (!verificar(p, TK_QUE))
        return error_en(p, actual(p), "falta 'que': la frase completa para declarar es 'fijate que' (se encontró %s)",
                        describir(actual(p), buf, sizeof buf));
    avanzar(p);

    const Token *nombre = leer_nombre(p, "variable", "después de 'fijate que'");
    if (!nombre) return NULL;

    Nodo *v = NULL;
    if (verificar(p, TK_ES)) {
        avanzar(p);
        v = parsear_expresion(p, "después de 'es'");
        if (!v) return NULL;
    } else if (verificar(p, TK_IDENT)) {
        const char *extra = actual(p)->lexema;
        return error_en(p, actual(p),
                        "el nombre de una variable no puede tener espacios ('%s %s'); use por ejemplo '%s_%s'",
                        nombre->lexema, extra, nombre->lexema, extra);
    }

    Nodo *n = nodo_con_nombre(NODO_DECLARACION, inicio->linea, nombre->lexema);
    n->valor = v;
    return n;
}

/* Semántica: el nombre no puede estar ya usado en el mismo ámbito */
void verificar_declaracion(Semantico *sem, Nodo *nodo) {
    /* El valor se revisa antes de declarar: 'fijate que x es x' es un error */
    TipoRP tipo = nodo->valor ? tipo_expresion(sem, nodo->valor) : RP_NI_MAIZ;
    Simbolo *previo = ts_buscar_en_ambito(sem->tabla, nodo->nombre);
    if (previo && previo->clase == SIM_CHAMBA) {
        error_semantico(sem, nodo->linea, "ya existe una chamba llamada '%s' (línea %d); use otro nombre para la "
                        "variable", nodo->nombre, previo->linea_declaracion);
        return;
    }
    if (previo && previo->clase == SIM_PARAMETRO) {
        error_semantico(sem, nodo->linea, "'%s' ya es un parámetro de la chamba '%s'; para cambiar su valor use "
                        "'ponele <valor> a %s'", nodo->nombre, sem->chamba_actual, nodo->nombre);
        return;
    }
    if (previo) {
        error_semantico(sem, nodo->linea, "la variable '%s' ya fue declarada en la línea %d; para cambiar su valor "
                        "use 'ponele <valor> a %s'", nodo->nombre, previo->linea_declaracion, nodo->nombre);
        return;
    }
    anotar(nodo, tipo);
    ts_declarar(sem->tabla, nodo->nombre, tipo, nodo->linea);
}

/* Ejecución: crea la celda de la variable en la memoria */
void ejecutar_declaracion(Interprete *in, const Nodo *nodo) {
    ValorRP v = nodo->valor ? evaluar(in, nodo->valor) : NADA;
    nueva_celda(in, nodo->nombre)->valor = v;  /* si falló queda en ni maiz, pero la variable existe */
}
