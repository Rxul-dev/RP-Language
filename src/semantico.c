#include "semantico.h"

static void anotar(Nodo *nodo, TipoRP tipo) {
    nodo->tipo = tipo;
    nodo->tiene_tipo = 1;
}

/* Busca una variable que se va a leer y registra el uso */
static Simbolo *usar(Semantico *sem, const char *nombre, int linea) {
    Simbolo *s = ts_buscar(sem->tabla, nombre);
    if (!s) {
        errores_agregar(sem->errores, FASE_SEMANTICA, linea, 0,
                        "la variable '%s' no ha sido declarada; declárela antes con 'fijate que %s'",
                        nombre, nombre);
        return NULL;
    }
    enteros_agregar(&s->usos, linea);
    return s;
}

/* Inferencia de tipo de un valor: el literal ya trae su tipo; una variable tiene el de la tabla */
static TipoRP tipo_de(Semantico *sem, Nodo *valor) {
    if (valor->clase == NODO_LITERAL) return valor->tipo;
    Simbolo *s = usar(sem, valor->nombre, valor->linea);
    anotar(valor, s ? s->tipo : RP_DESCONOCIDO);
    return valor->tipo;
}

static void declaracion(Semantico *sem, Nodo *nodo) {
    /* El valor se revisa antes de declarar: 'fijate que x es x' es un error */
    TipoRP tipo = nodo->valor ? tipo_de(sem, nodo->valor) : RP_NI_MAIZ;
    Simbolo *previo = ts_buscar(sem->tabla, nodo->nombre);
    if (previo) {
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0,
                        "la variable '%s' ya fue declarada en la línea %d; para cambiar su valor use "
                        "'ponele <valor> a %s'", nodo->nombre, previo->linea_declaracion, nodo->nombre);
        return;
    }
    anotar(nodo, tipo);
    ts_declarar(sem->tabla, nodo->nombre, tipo, nodo->linea);
}

static void asignacion(Semantico *sem, Nodo *nodo) {
    TipoRP tipo = tipo_de(sem, nodo->valor);
    Simbolo *s = ts_buscar(sem->tabla, nodo->nombre);
    if (!s) {
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0,
                        "no se puede asignar a '%s' porque no ha sido declarada; declárela primero con "
                        "'fijate que %s'", nodo->nombre, nodo->nombre);
        return;
    }
    anotar(nodo, tipo);
    s->tipo = tipo;  /* tipado dinámico: la variable adopta el tipo del nuevo valor */
    enteros_agregar(&s->asignaciones, nodo->linea);
    if (s->historial.datos[s->historial.cantidad - 1] != (int)tipo) enteros_agregar(&s->historial, (int)tipo);
}

static void consulta_tipo(Semantico *sem, Nodo *nodo) {
    Simbolo *s = usar(sem, nodo->nombre, nodo->linea);
    if (s) anotar(nodo, s->tipo);
}

void semantico_visitar(Semantico *sem, Nodo *sentencia) {
    switch (sentencia->clase) {
        case NODO_DECLARACION: declaracion(sem, sentencia); break;
        case NODO_ASIGNACION: asignacion(sem, sentencia); break;
        case NODO_CONSULTA_TIPO: consulta_tipo(sem, sentencia); break;
        default: break;
    }
}

void semantico_analizar(Semantico *sem, Nodo *programa) {
    for (int i = 0; i < programa->num_sentencias; i++) semantico_visitar(sem, programa->sentencias[i]);
}
