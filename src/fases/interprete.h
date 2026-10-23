/* Intérprete: ejecuta el AST ya validado por el analizador semántico.
 *
 * La memoria es una pila de celdas. Al llamar una chamba se abre un "marco":
 * sus parámetros y variables locales se apilan encima y se liberan al terminar.
 * Desde una chamba solo se ven su marco y las variables globales.
 *
 * Lo que hace cada instrucción está en su archivo de src/instrucciones/.
 */
#ifndef INTERPRETE_H
#define INTERPRETE_H

#include "base/errores.h"
#include "base/valor.h"
#include "fases/ast.h"

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

/* ------------------------------------------------ para la ejecución de cada instrucción */

/* El valor "ni maiz" */
#define NADA ((ValorRP){.tipo = RP_NI_MAIZ})

/* Una variable declarada dentro de un 'si' solo existe si el 'si' se cumplió */
#define NO_EXISTE "la variable '%s' no existe (¿se declaró dentro de un 'si' que no se cumplió?)"

/* Guarda el primer error de ejecución; desde ahí las sentencias dejan de ejecutarse */
void error_ejecucion(Interprete *in, int linea, const char *formato, ...);
/* Busca en el marco de la chamba actual y luego en las globales; NULL si no existe */
Celda *buscar_variable(Interprete *in, const char *nombre);
/* Crea una variable en el marco actual con valor ni maiz */
Celda *nueva_celda(Interprete *in, const char *nombre);
/* Libera las celdas desde `inicio` hasta el tope de la pila */
void liberar_hasta(Interprete *in, int inicio);
/* Ejecuta un cuerpo hasta el final, un 'devolveme' o un error */
void ejecutar_cuerpo(Interprete *in, const Nodo *bloque);

#endif
