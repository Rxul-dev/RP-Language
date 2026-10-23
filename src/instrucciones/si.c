/* SI:  si edad >= 18 ... ya estuvo
 *
 * Ejecuta las líneas del bloque solo si la condición da simon.
 * Equivale a 'if edad >= 18:' en Python.
 */
#include "instrucciones/instrucciones.h"

#include <stdio.h>
#include <stdlib.h>

#include "base/util.h"

/* Sintaxis:  si → 'si' expresion SALTO bloque */
Nodo *parsear_si(Parser *p) {
    const Token *inicio = avanzar(p);
    char buf[200], que[100];
    Nodo *condicion = parsear_expresion(p, "después de 'si'");
    if (!condicion) {
        saltar_bloque(p);
        return NULL;
    }
    if (!verificar(p, TK_SALTO)) {
        error_en(p, actual(p), "después de la condición del 'si' va un salto de línea; lo que se hace "
                 "se escribe en las líneas siguientes (sobra %s)", describir(actual(p), buf, sizeof buf));
        ast_liberar(condicion);
        saltar_bloque(p);
        return NULL;
    }
    Nodo *n = nodo_nuevo(NODO_SI, inicio->linea);
    n->valor = condicion;

    snprintf(que, sizeof que, "el 'si' de la línea %d", inicio->linea);
    p->si_abiertos++;
    int ok = bloque(p, n, que);
    p->si_abiertos--;
    if (ok) return n;
    ast_liberar(n);
    return NULL;
}

/* Semántica: la condición debe dar simon o nel */
void verificar_si(Semantico *sem, Nodo *nodo) {
    TipoRP tipo = tipo_expresion(sem, nodo->valor);
    if (tipo != RP_DESCONOCIDO && tipo != RP_SIMONEL)
        error_semantico(sem, nodo->linea, "la condición del 'si' debe dar simon o nel, pero da un valor de tipo %s; "
                        "use una comparación, por ejemplo 'edad >= 18'", tipo_rp_nombre(tipo));

    /* El cuerpo puede no ejecutarse: si adentro una variable cambia de tipo, después del 'si'
     * su tipo se sabe hasta ejecutar. Se guardan los tipos de antes para comparar. */
    int antes = sem->tabla->cantidad;
    TipoRP *tipos = xcalloc(antes ? (size_t)antes : 1, sizeof *tipos);
    for (int i = 0; i < antes; i++) tipos[i] = sem->tabla->orden[i]->tipo;

    semantico_analizar(sem, nodo);  /* revisa el cuerpo igual que un programa */

    for (int i = 0; i < antes; i++) {
        Simbolo *s = sem->tabla->orden[i];
        if (s->tipo == tipos[i]) continue;
        s->tipo = RP_DESCONOCIDO;
        if (s->historial.datos[s->historial.cantidad - 1] != RP_DESCONOCIDO)
            enteros_agregar(&s->historial, RP_DESCONOCIDO);
    }
    free(tipos);
}

/* Ejecución: si es simon se ejecuta el cuerpo; un 'devolveme' adentro termina también la chamba */
void ejecutar_si(Interprete *in, const Nodo *nodo) {
    ValorRP condicion = evaluar(in, nodo->valor);
    if (in->hay_error) {
        valor_liberar(&condicion);
        return;
    }
    if (condicion.tipo != RP_SIMONEL) {
        error_ejecucion(in, nodo->linea, "la condición del 'si' debe dar simon o nel, pero dio un valor "
                        "de tipo %s", tipo_rp_nombre(condicion.tipo));
        valor_liberar(&condicion);
        return;
    }
    if (condicion.dato.simonel) ejecutar_cuerpo(in, nodo);
}
