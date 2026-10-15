/* Intérprete: ejecuta el AST ya validado por el analizador semántico.
 *
 * La memoria es una pila de celdas. Al llamar una chamba se abre un "marco":
 * sus parámetros y variables locales se apilan encima y se liberan al terminar.
 * Desde una chamba solo se ven su marco y las variables globales.
 */
#ifndef INTERPRETE_H
#define INTERPRETE_H

#include "ast.h"
#include "errores.h"
#include "valor.h"

#define MAX_PROFUNDIDAD 1000  /* llamadas anidadas antes de detener una recursión sin fin */

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
    int marco;          /* primera celda de la chamba que se está ejecutando (0 afuera de toda chamba) */
    int globales;       /* cuántas celdas son globales mientras se ejecuta una chamba */
    int profundidad;    /* chambas ejecutándose una dentro de otra */

    const Nodo **chambas;  /* chambas declaradas hasta ahora */
    int num_chambas;
    int capacidad_chambas;

    int devolviendo;    /* 1 después de un 'devolveme': se dejan de ejecutar las sentencias de la chamba */
    ValorRP devuelto;

    int hay_error;      /* un error de ejecución detiene el programa */
    ErrorRP error;

    FuncionSalida salida;
    void *contexto;
} Interprete;

void interprete_iniciar(Interprete *in, FuncionSalida salida, void *contexto); /* salida NULL = pantalla */
int interprete_ejecutar(Interprete *in, const Nodo *programa);                /* 0 si hubo error */
int interprete_ejecutar_sentencia(Interprete *in, const Nodo *sentencia);     /* 0 si hubo error */
void interprete_liberar(Interprete *in);

#endif
