/* Muestra en consola el resultado de cada fase. */
#ifndef REPORTES_H
#define REPORTES_H

#include "base/errores.h"
#include "fases/ast.h"
#include "fases/interprete.h"
#include "fases/tabla_simbolos.h"
#include "fases/token.h"

void reporte_titulo(const char *texto);
void reporte_codigo(const char *fuente);
void reporte_tokens(const ListaTokens *tokens);
void reporte_arbol_ast(const Nodo *programa);
void reporte_tabla_ast(const Nodo *programa);
void reporte_tabla_simbolos(const TablaSimbolos *tabla);
void reporte_memoria(const Interprete *in);
void reporte_errores(const ListaErrores *errores);
void reporte_error(const ErrorRP *error);

#endif
