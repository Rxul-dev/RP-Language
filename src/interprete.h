/* Intérprete: ejecuta el AST ya validado por el analizador semántico. */
#ifndef INTERPRETE_H
#define INTERPRETE_H

#include "ast.h"
#include "valor.h"

typedef struct {
    char *nombre;
    ValorRP valor;
} Celda;

/* Recibe cada línea que imprime el programa (permite capturar la salida en las pruebas) */
typedef void (*FuncionSalida)(void *contexto, const char *texto);

typedef struct {
    Celda *memoria;
    int cantidad;
    int capacidad;
    FuncionSalida salida;
    void *contexto;
} Interprete;

void interprete_iniciar(Interprete *in, FuncionSalida salida, void *contexto); /* salida NULL = pantalla */
void interprete_ejecutar(Interprete *in, const Nodo *programa);
void interprete_ejecutar_sentencia(Interprete *in, const Nodo *sentencia);
void interprete_liberar(Interprete *in);

#endif
