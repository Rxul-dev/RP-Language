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
 *   Una función por cada clase de instrucción: declaracion(), asignacion(),
 *   chamba(), devolveme()... semantico_visitar() elige cuál usar.
 */
#include "semantico.h"

#include <stdlib.h>

static void anotar(Nodo *nodo, TipoRP tipo) {
    nodo->tipo = tipo;
    nodo->tiene_tipo = 1;
}

static int es_global(const Simbolo *s) {
    return s->ambito[0] == 'g';  /* "global"; los demás ámbitos empiezan con "chamba " */
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
    if (s->clase == SIM_CHAMBA) {
        errores_agregar(sem->errores, FASE_SEMANTICA, linea, 0,
                        "'%s' es una chamba, no una variable; para usarla llámela con '%s(...)'", nombre, nombre);
        return NULL;
    }
    enteros_agregar(&s->usos, linea);
    return s;
}

static TipoRP tipo_de(Semantico *sem, Nodo *valor);

/* llamada → la chamba debe existir y recibir tantos valores como parámetros tiene */
static TipoRP llamada(Semantico *sem, Nodo *nodo) {
    for (int i = 0; i < nodo->num_parametros; i++) tipo_de(sem, nodo->parametros[i]);

    Simbolo *s = ts_buscar(sem->tabla, nodo->nombre);
    if (!s) {
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0,
                        "la chamba '%s' no ha sido declarada; declárela antes con 'chamba %s(...)'",
                        nodo->nombre, nodo->nombre);
    } else if (s->clase != SIM_CHAMBA) {
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0, "'%s' es %s, no una chamba; no se puede llamar",
                        nodo->nombre, s->clase == SIM_PARAMETRO ? "un parámetro" : "una variable");
    } else {
        enteros_agregar(&s->usos, nodo->linea);
        if (s->num_parametros != nodo->num_parametros)
            errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0,
                            "la chamba '%s' recibe %d valor(es), pero se le pasaron %d", nodo->nombre,
                            s->num_parametros, nodo->num_parametros);
    }
    /* Con tipado dinámico lo que devuelve una chamba se sabe hasta ejecutarla */
    anotar(nodo, RP_DESCONOCIDO);
    return RP_DESCONOCIDO;
}

static TipoRP operacion(Semantico *sem, Nodo *nodo) {
    TipoRP izq = tipo_de(sem, nodo->izq);
    TipoRP der = tipo_de(sem, nodo->der);
    TipoRP resultado = RP_DESCONOCIDO;
    if (izq != RP_DESCONOCIDO && der != RP_DESCONOCIDO) {
        if (!tipo_operacion(nodo->operador, izq, der, &resultado)) {
            char mensaje[300];
            mensaje_operacion_invalida(nodo->operador, izq, der, mensaje, sizeof mensaje);
            errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0, "%s", mensaje);
            resultado = RP_DESCONOCIDO;
        }
    } else if (operador_es_comparacion(nodo->operador)) {
        /* Si un lado es dinámico la operación se revisa al ejecutar, pero una comparación siempre da simonel */
        resultado = RP_SIMONEL;
    }
    anotar(nodo, resultado);
    return resultado;
}

/* Inferencia de tipo de una expresión: el literal ya trae su tipo; una variable tiene el de la tabla */
static TipoRP tipo_de(Semantico *sem, Nodo *valor) {
    switch (valor->clase) {
        case NODO_LITERAL: return valor->tipo;
        case NODO_OPERACION: return operacion(sem, valor);
        case NODO_LLAMADA: return llamada(sem, valor);
        default: break;
    }
    Simbolo *s = usar(sem, valor->nombre, valor->linea);
    /* Dentro de una chamba una variable global puede tener otro tipo cuando se llame la chamba */
    int dinamico = !s || (sem->chamba_actual && es_global(s));
    anotar(valor, dinamico ? RP_DESCONOCIDO : s->tipo);
    return valor->tipo;
}

static void declaracion(Semantico *sem, Nodo *nodo) {
    /* El valor se revisa antes de declarar: 'fijate que x es x' es un error */
    TipoRP tipo = nodo->valor ? tipo_de(sem, nodo->valor) : RP_NI_MAIZ;
    Simbolo *previo = ts_buscar_en_ambito(sem->tabla, nodo->nombre);
    if (previo && previo->clase == SIM_CHAMBA) {
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0,
                        "ya existe una chamba llamada '%s' (línea %d); use otro nombre para la variable",
                        nodo->nombre, previo->linea_declaracion);
        return;
    }
    if (previo && previo->clase == SIM_PARAMETRO) {
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0,
                        "'%s' ya es un parámetro de la chamba '%s'; para cambiar su valor use 'ponele <valor> a %s'",
                        nodo->nombre, sem->chamba_actual, nodo->nombre);
        return;
    }
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
    if (s->clase == SIM_CHAMBA) {
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0,
                        "'%s' es una chamba; con 'ponele' solo se cambia el valor de una variable", nodo->nombre);
        return;
    }
    /* Una chamba lee las variables globales pero no las cambia: así su tipo en la tabla sigue siendo
     * cierto y el resultado de la chamba sale solo por 'devolveme' */
    if (sem->chamba_actual && es_global(s)) {
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0,
                        "la chamba '%s' no puede cambiar la variable global '%s'; devuelva el valor con "
                        "'devolveme' y asígnelo afuera", sem->chamba_actual, nodo->nombre);
        return;
    }
    anotar(nodo, tipo);
    s->tipo = tipo;  /* tipado dinámico: la variable adopta el tipo del nuevo valor */
    enteros_agregar(&s->asignaciones, nodo->linea);
    if (s->historial.datos[s->historial.cantidad - 1] != (int)tipo) enteros_agregar(&s->historial, (int)tipo);
}

static void consulta_tipo(Semantico *sem, Nodo *nodo) {
    Simbolo *s = usar(sem, nodo->nombre, nodo->linea);
    if (s) anotar(nodo, sem->chamba_actual && es_global(s) ? RP_DESCONOCIDO : s->tipo);
}

static void contame(Semantico *sem, Nodo *nodo) {
    anotar(nodo, tipo_de(sem, nodo->valor));
}

static void devolveme(Semantico *sem, Nodo *nodo) {
    TipoRP tipo = nodo->valor ? tipo_de(sem, nodo->valor) : RP_NI_MAIZ;
    if (!sem->chamba_actual) {
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0,
                        "'devolveme' solo se puede usar dentro de una chamba");
        return;
    }
    anotar(nodo, tipo);
}

/* si → la condición debe dar simon o nel */
static void si(Semantico *sem, Nodo *nodo) {
    TipoRP tipo = tipo_de(sem, nodo->valor);
    if (tipo != RP_DESCONOCIDO && tipo != RP_SIMONEL)
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0,
                        "la condición del 'si' debe dar simon o nel, pero da un valor de tipo %s; "
                        "use una comparación, por ejemplo 'edad >= 18'", tipo_rp_nombre(tipo));

    /* El cuerpo puede no ejecutarse: si adentro una variable cambia de tipo, después del 'si'
     * su tipo se sabe hasta ejecutar. Se guardan los tipos de antes para comparar. */
    int antes = sem->tabla->cantidad;
    TipoRP *tipos = xcalloc(antes ? (size_t)antes : 1, sizeof *tipos);
    for (int i = 0; i < antes; i++) tipos[i] = sem->tabla->orden[i]->tipo;

    for (int i = 0; i < nodo->num_sentencias; i++) semantico_visitar(sem, nodo->sentencias[i]);

    for (int i = 0; i < antes; i++) {
        Simbolo *s = sem->tabla->orden[i];
        if (s->tipo == tipos[i]) continue;
        s->tipo = RP_DESCONOCIDO;
        if (s->historial.datos[s->historial.cantidad - 1] != RP_DESCONOCIDO)
            enteros_agregar(&s->historial, RP_DESCONOCIDO);
    }
    free(tipos);
}

/* chamba → se declara en el ámbito global y su cuerpo se revisa en un ámbito propio */
static void chamba(Semantico *sem, Nodo *nodo) {
    Simbolo *previo = ts_buscar_en_ambito(sem->tabla, nodo->nombre);
    if (previo) {
        errores_agregar(sem->errores, FASE_SEMANTICA, nodo->linea, 0, "ya existe %s llamada '%s' (línea %d)",
                        previo->clase == SIM_CHAMBA ? "una chamba" : "una variable", nodo->nombre,
                        previo->linea_declaracion);
        return;
    }
    /* Se declara antes de revisar el cuerpo para que la chamba pueda llamarse a sí misma */
    Simbolo *s = ts_declarar(sem->tabla, nodo->nombre, RP_DESCONOCIDO, nodo->linea);
    s->clase = SIM_CHAMBA;
    s->num_parametros = nodo->num_parametros;

    ts_entrar_chamba(sem->tabla, nodo->nombre);
    sem->chamba_actual = nodo->nombre;
    for (int i = 0; i < nodo->num_parametros; i++) {
        Nodo *param = nodo->parametros[i];
        if (ts_buscar_en_ambito(sem->tabla, param->nombre)) {
            errores_agregar(sem->errores, FASE_SEMANTICA, param->linea, 0,
                            "el parámetro '%s' está repetido en la chamba '%s'", param->nombre, nodo->nombre);
            continue;
        }
        Simbolo *p = ts_declarar(sem->tabla, param->nombre, RP_DESCONOCIDO, param->linea);
        p->clase = SIM_PARAMETRO;
        anotar(param, RP_DESCONOCIDO);
    }
    for (int i = 0; i < nodo->num_sentencias; i++) semantico_visitar(sem, nodo->sentencias[i]);
    sem->chamba_actual = NULL;
    ts_salir_chamba(sem->tabla);
}

void semantico_visitar(Semantico *sem, Nodo *sentencia) {
    switch (sentencia->clase) {
        case NODO_DECLARACION: declaracion(sem, sentencia); break;
        case NODO_ASIGNACION: asignacion(sem, sentencia); break;
        case NODO_CONSULTA_TIPO: consulta_tipo(sem, sentencia); break;
        case NODO_CONTAME: contame(sem, sentencia); break;
        case NODO_DEVOLVEME: devolveme(sem, sentencia); break;
        case NODO_SI: si(sem, sentencia); break;
        case NODO_CHAMBA: chamba(sem, sentencia); break;
        case NODO_LLAMADA: llamada(sem, sentencia); break;
        default: break;
    }
}

void semantico_analizar(Semantico *sem, Nodo *programa) {
    for (int i = 0; i < programa->num_sentencias; i++) semantico_visitar(sem, programa->sentencias[i]);
}
