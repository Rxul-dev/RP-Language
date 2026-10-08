/* Muestra en consola el resultado de cada fase. */
#ifndef REPORTES_H
#define REPORTES_H

#include "ast.h"
#include "errores.h"
#include "interprete.h"
#include "tabla_simbolos.h"
#include "token.h"

void reporte_titulo(const char *texto);
void reporte_codigo(const char *fuente);
void reporte_tokens(const ListaTokens *tokens);
void reporte_arbol_ast(const Nodo *programa);
void reporte_tabla_ast(const Nodo *programa);
void reporte_tabla_simbolos(const TablaSimbolos *tabla);
void reporte_memoria(const Interprete *in);
void reporte_errores(const ListaErrores *errores);

#endif
