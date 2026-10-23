/* CHAMBAS (funciones):
 *
 *     chamba sumar(x, y)          declara la chamba     (def sumar(x, y): en Python)
 *         devolveme x + y         devuelve un valor     (return x + y)
 *     ya estuvo
 *     contame sumar(1, 2)         la llama              (sumar(1, 2))
 *
 * Los parámetros y las variables de adentro son locales: se crean al llamar la
 * chamba y se borran cuando termina.
 */
#include "instrucciones/instrucciones.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/util.h"

/* ============================================================ chamba sumar(x, y) ... ya estuvo */

static Nodo *parsear_parametro(Parser *p, const char *contexto) {
    const Token *t = leer_nombre(p, "parámetro", contexto);
    return t ? nodo_con_nombre(NODO_PARAMETRO, t->linea, t->lexema) : NULL;
}

/* Sintaxis:  chamba → 'chamba' IDENT '(' [ IDENT { ',' IDENT } ] ')' SALTO bloque */
Nodo *parsear_chamba(Parser *p) {
    const Token *inicio = avanzar(p);
    char buf[200], que[100];
    if (p->chamba_abierta)
        return error_en(p, inicio, "no se puede declarar una chamba dentro de otra; "
                        "¿olvidó cerrar '%s' con 'ya estuvo'?", p->chamba_abierta->lexema);
    if (p->si_abiertos)
        return error_en(p, inicio, "las chambas se declaran afuera de los bloques 'si'; "
                        "¿olvidó cerrar el 'si' con 'ya estuvo'?");

    const Token *nombre = leer_nombre(p, "chamba", "después de 'chamba'");
    if (!nombre) {
        saltar_bloque(p);
        return NULL;
    }
    Nodo *n = nodo_con_nombre(NODO_CHAMBA, inicio->linea, nombre->lexema);
    snprintf(que, sizeof que, "la chamba '%s'", nombre->lexema);

    if (!verificar(p, TK_PAR_ABRE)) {
        error_en(p, actual(p), "falta '(' después del nombre: escriba 'chamba %s(<parámetros>)' "
                 "(se encontró %s)", nombre->lexema, describir(actual(p), buf, sizeof buf));
        goto error;
    }
    avanzar(p);
    if (!leer_lista(p, n, TK_PAR_CIERRA, parsear_parametro, que)) goto error;
    if (!verificar(p, TK_SALTO)) {
        error_en(p, actual(p), "después de 'chamba %s(...)' va un salto de línea; el cuerpo se escribe "
                 "en las líneas siguientes (sobra %s)", nombre->lexema, describir(actual(p), buf, sizeof buf));
        goto error;
    }

    p->chamba_abierta = nombre;
    int ok = bloque(p, n, que);
    p->chamba_abierta = NULL;
    if (ok) return n;
    ast_liberar(n);
    return NULL;

error:
    ast_liberar(n);
    saltar_bloque(p);
    return NULL;
}

/* Semántica: se declara en el ámbito global y su cuerpo se revisa en un ámbito propio */
void verificar_chamba(Semantico *sem, Nodo *nodo) {
    Simbolo *previo = ts_buscar_en_ambito(sem->tabla, nodo->nombre);
    if (previo) {
        error_semantico(sem, nodo->linea, "ya existe %s llamada '%s' (línea %d)",
                        previo->clase == SIM_CHAMBA ? "una chamba" : "una variable", nodo->nombre,
                        previo->linea_declaracion);
        return;
    }
    /* Se declara antes de revisar el cuerpo para que la chamba pueda llamarse a sí misma */
    Simbolo *s = ts_declarar(sem->tabla, nodo->nombre, RP_DESCONOCIDO, nodo->linea);
    s->clase = SIM_CHAMBA;
    s->num_parametros = nodo->num_elementos;

    ts_entrar_chamba(sem->tabla, nodo->nombre);
    sem->chamba_actual = nodo->nombre;
    for (int i = 0; i < nodo->num_elementos; i++) {
        Nodo *param = nodo->elementos[i];
        if (ts_buscar_en_ambito(sem->tabla, param->nombre)) {
            error_semantico(sem, param->linea, "el parámetro '%s' está repetido en la chamba '%s'", param->nombre,
                            nodo->nombre);
            continue;
        }
        ts_declarar(sem->tabla, param->nombre, RP_DESCONOCIDO, param->linea)->clase = SIM_PARAMETRO;
        anotar(param, RP_DESCONOCIDO);
    }
    semantico_analizar(sem, nodo);  /* revisa el cuerpo igual que un programa */
    sem->chamba_actual = NULL;
    ts_salir_chamba(sem->tabla);
}

static const Nodo *buscar_chamba(Interprete *in, const char *nombre) {
    for (int i = 0; i < in->num_chambas; i++) {
        if (strcmp(in->chambas[i]->nombre, nombre) == 0) return in->chambas[i];
    }
    return NULL;
}

/* Ejecución: solo se registra; el cuerpo se ejecuta hasta que alguien la llame */
void ejecutar_chamba(Interprete *in, const Nodo *nodo) {
    in->chambas = crecer(in->chambas, in->num_chambas, &in->capacidad_chambas, sizeof *in->chambas);
    in->chambas[in->num_chambas++] = nodo;
}

/* ============================================================ devolveme x + y */

/* Sintaxis:  devolveme → 'devolveme' [ expresion ] */
Nodo *parsear_devolveme(Parser *p) {
    TipoToken despues = siguiente(p)->tipo;
    if (despues == TK_SALTO || despues == TK_FIN) return nodo_nuevo(NODO_DEVOLVEME, avanzar(p)->linea);
    return instruccion_con_valor(p, NODO_DEVOLVEME);
}

/* Semántica: solo tiene sentido dentro de una chamba */
void verificar_devolveme(Semantico *sem, Nodo *nodo) {
    TipoRP tipo = nodo->valor ? tipo_expresion(sem, nodo->valor) : RP_NI_MAIZ;
    if (!sem->chamba_actual) {
        error_semantico(sem, nodo->linea, "'devolveme' solo se puede usar dentro de una chamba");
        return;
    }
    anotar(nodo, tipo);
}

/* Ejecución: guarda el valor; las demás sentencias de la chamba ya no se ejecutan */
void ejecutar_devolveme(Interprete *in, const Nodo *nodo) {
    ValorRP v = nodo->valor ? evaluar(in, nodo->valor) : NADA;
    if (in->hay_error) {
        valor_liberar(&v);
        return;
    }
    in->devuelto = v;
    in->devolviendo = 1;
}

/* ============================================================ sumar(1, 2) */

/* Sintaxis:  llamada → IDENT '(' [ expresion { ',' expresion } ] ')' */
Nodo *parsear_llamada(Parser *p) {
    const Token *nombre = avanzar(p);
    char que[300];
    avanzar(p);  /* '(' */
    Nodo *n = nodo_con_nombre(NODO_LLAMADA, nombre->linea, nombre->lexema);
    snprintf(que, sizeof que, "la llamada a '%s'", nombre->lexema);
    if (leer_lista(p, n, TK_PAR_CIERRA, parsear_expresion, que)) return n;
    ast_liberar(n);
    return NULL;
}

/* Semántica: la chamba debe existir y recibir tantos valores como parámetros tiene */
TipoRP tipo_llamada(Semantico *sem, Nodo *nodo) {
    for (int i = 0; i < nodo->num_elementos; i++) tipo_expresion(sem, nodo->elementos[i]);

    Simbolo *s = ts_buscar(sem->tabla, nodo->nombre);
    if (!s) {
        error_semantico(sem, nodo->linea, "la chamba '%s' no ha sido declarada; declárela antes con 'chamba %s(...)'",
                        nodo->nombre, nodo->nombre);
    } else if (s->clase != SIM_CHAMBA) {
        error_semantico(sem, nodo->linea, "'%s' es %s, no una chamba; no se puede llamar", nodo->nombre,
                        s->clase == SIM_PARAMETRO ? "un parámetro" : "una variable");
    } else {
        enteros_agregar(&s->usos, nodo->linea);
        if (s->num_parametros != nodo->num_elementos)
            error_semantico(sem, nodo->linea, "la chamba '%s' recibe %d valor(es), pero se le pasaron %d",
                            nodo->nombre, s->num_parametros, nodo->num_elementos);
    }
    /* Con tipado dinámico lo que devuelve una chamba se sabe hasta ejecutarla */
    anotar(nodo, RP_DESCONOCIDO);
    return RP_DESCONOCIDO;
}

/* Ejecución: los argumentos se evalúan en el ámbito de quien llama, luego se abre un marco
 * nuevo con los parámetros y se ejecuta el cuerpo hasta el primer 'devolveme' */
ValorRP evaluar_llamada(Interprete *in, const Nodo *llamada) {
    const Nodo *chamba = buscar_chamba(in, llamada->nombre);
    if (!chamba) {
        error_ejecucion(in, llamada->linea, "la chamba '%s' no existe", llamada->nombre);
        return NADA;
    }
    if (chamba->num_elementos != llamada->num_elementos) {
        error_ejecucion(in, llamada->linea, "la chamba '%s' recibe %d valor(es), pero se le pasaron %d",
                        chamba->nombre, chamba->num_elementos, llamada->num_elementos);
        return NADA;
    }
    if (in->profundidad >= MAX_PROFUNDIDAD) {
        error_ejecucion(in, llamada->linea, "la chamba '%s' se llamó más de %d veces una dentro de otra; "
                        "revise que no se esté llamando a sí misma sin fin", chamba->nombre, MAX_PROFUNDIDAD);
        return NADA;
    }

    int n = llamada->num_elementos;
    ValorRP *argumentos = xcalloc(n ? (size_t)n : 1, sizeof *argumentos);
    for (int i = 0; i < n && !in->hay_error; i++) argumentos[i] = evaluar(in, llamada->elementos[i]);
    if (in->hay_error) {
        for (int i = 0; i < n; i++) valor_liberar(&argumentos[i]);
        free(argumentos);
        return NADA;
    }

    int marco_anterior = in->marco, globales_anterior = in->globales;
    if (in->profundidad == 0) in->globales = in->cantidad;
    in->marco = in->cantidad;
    in->profundidad++;
    for (int i = 0; i < n; i++) nueva_celda(in, chamba->elementos[i]->nombre)->valor = argumentos[i];
    free(argumentos);

    ejecutar_cuerpo(in, chamba);

    /* Sin 'devolveme' la chamba devuelve ni maiz */
    ValorRP resultado = NADA;
    if (in->devolviendo) {
        resultado = in->devuelto;
        in->devuelto = NADA;
        in->devolviendo = 0;
    }
    liberar_hasta(in, in->marco);
    in->marco = marco_anterior;
    in->globales = globales_anterior;
    in->profundidad--;
    return resultado;
}
