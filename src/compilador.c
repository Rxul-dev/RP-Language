#include "compilador.h"

#include <string.h>

#include "lexico.h"
#include "semantico.h"
#include "sintactico.h"

void compilar(const char *fuente, Resultado *r) {
    memset(r, 0, sizeof *r);
    ts_iniciar(&r->tabla);

    lexico_analizar(fuente, 1, &r->tokens, &r->errores_lexicos);
    r->programa = sintactico_analizar(&r->tokens, &r->errores_sintacticos);

    /* El semántico solo corre sobre un AST completo; con errores previos aparecerían
     * errores falsos (p. ej. variables "no declaradas" en líneas que se descartaron). */
    if (resultado_ok(r)) {
        Semantico sem = {&r->tabla, &r->errores_semanticos, NULL};
        semantico_analizar(&sem, r->programa);
        r->semantico_ejecutado = 1;
    }
}

int resultado_ok(const Resultado *r) {
    return r->errores_lexicos.cantidad + r->errores_sintacticos.cantidad + r->errores_semanticos.cantidad == 0;
}

void resultado_todos_los_errores(const Resultado *r, ListaErrores *destino) {
    errores_unir(destino, &r->errores_lexicos);
    errores_unir(destino, &r->errores_sintacticos);
    errores_unir(destino, &r->errores_semanticos);
    errores_ordenar(destino);
}

void resultado_liberar(Resultado *r) {
    tokens_liberar(&r->tokens);
    ast_liberar(r->programa);
    ts_liberar(&r->tabla);
    errores_liberar(&r->errores_lexicos);
    errores_liberar(&r->errores_sintacticos);
    errores_liberar(&r->errores_semanticos);
}
