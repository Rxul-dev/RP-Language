/* Une las fases del compilador: léxico → sintáctico → semántico. */
#ifndef COMPILADOR_H
#define COMPILADOR_H

#include "ast.h"
#include "errores.h"
#include "tabla_simbolos.h"
#include "token.h"

typedef struct {
    ListaTokens tokens;
    Nodo *programa;
    TablaSimbolos tabla;
    ListaErrores errores_lexicos;
    ListaErrores errores_sintacticos;
    ListaErrores errores_semanticos;
    int semantico_ejecutado;
} Resultado;

void compilar(const char *fuente, Resultado *r);
int resultado_ok(const Resultado *r);
void resultado_todos_los_errores(const Resultado *r, ListaErrores *destino); /* ordenados por línea */
void resultado_liberar(Resultado *r);

#endif
