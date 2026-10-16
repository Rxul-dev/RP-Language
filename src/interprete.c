/* INTÉRPRETE (fase 4)
 *
 * ¿Qué hace?
 *   Ejecuta el programa: recorre el AST instrucción por instrucción y hace lo
 *   que cada una dice (crear variables, mostrar valores, llamar chambas...).
 *
 * ¿Cómo guarda las variables?
 *   En una lista llamada "memoria". Cada variable es una celda con su nombre y
 *   su valor. Cuando se llama una chamba, sus parámetros y variables se agregan
 *   al final de la lista y se borran cuando la chamba termina.
 *
 * ¿Cómo está ordenado?
 *   evaluar() calcula el valor de una expresión (5 + x, sumar(1, 2)...).
 *   llamar() ejecuta una chamba.
 *   interprete_ejecutar_sentencia() ejecuta una instrucción.
 */
#include "interprete.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static const ValorRP NADA = {RP_NI_MAIZ, {0}};

static void salida_pantalla(void *contexto, const char *texto) {
    (void)contexto;
    printf("%s\n", texto);
}

void interprete_iniciar(Interprete *in, FuncionSalida salida, void *contexto) {
    memset(in, 0, sizeof *in);
    in->salida = salida ? salida : salida_pantalla;
    in->contexto = contexto;
    in->devuelto = NADA;
}

/* Guarda el primer error de ejecución; desde ahí las sentencias dejan de ejecutarse */
static void error_ejecucion(Interprete *in, int linea, const char *formato, ...) {
    if (in->hay_error) return;
    in->hay_error = 1;
    memset(&in->error, 0, sizeof in->error);
    in->error.fase = FASE_EJECUCION;
    in->error.linea = linea;
    va_list args;
    va_start(args, formato);
    vsnprintf(in->error.mensaje, sizeof in->error.mensaje, formato, args);
    va_end(args);
}

/* Primero el marco de la chamba actual y, si se está dentro de una chamba, las globales */
static Celda *buscar(Interprete *in, const char *nombre) {
    for (int i = in->cantidad - 1; i >= in->marco; i--) {
        if (strcmp(in->memoria[i].nombre, nombre) == 0) return &in->memoria[i];
    }
    if (in->profundidad > 0) {
        for (int i = in->globales - 1; i >= 0; i--) {
            if (strcmp(in->memoria[i].nombre, nombre) == 0) return &in->memoria[i];
        }
    }
    return NULL;
}

static Celda *nueva_celda(Interprete *in, const char *nombre) {
    if (in->cantidad == in->capacidad) {
        in->capacidad = in->capacidad ? in->capacidad * 2 : 8;
        in->memoria = xrealloc(in->memoria, (size_t)in->capacidad * sizeof *in->memoria);
    }
    Celda *c = &in->memoria[in->cantidad++];
    c->nombre = xstrdup(nombre);
    c->valor.tipo = RP_NI_MAIZ;
    return c;
}

/* Libera las celdas desde `inicio` hasta el tope de la pila */
static void liberar_hasta(Interprete *in, int inicio) {
    while (in->cantidad > inicio) {
        Celda *c = &in->memoria[--in->cantidad];
        free(c->nombre);
        valor_liberar(&c->valor);
    }
}

static const Nodo *buscar_chamba(Interprete *in, const char *nombre) {
    for (int i = 0; i < in->num_chambas; i++) {
        if (strcmp(in->chambas[i]->nombre, nombre) == 0) return in->chambas[i];
    }
    return NULL;
}

static void registrar_chamba(Interprete *in, const Nodo *chamba) {
    if (in->num_chambas == in->capacidad_chambas) {
        in->capacidad_chambas = in->capacidad_chambas ? in->capacidad_chambas * 2 : 8;
        in->chambas = xrealloc(in->chambas, (size_t)in->capacidad_chambas * sizeof *in->chambas);
    }
    in->chambas[in->num_chambas++] = chamba;
}

static ValorRP llamar(Interprete *in, const Nodo *llamada);

/* Devuelve un valor nuevo (el chisme se duplica para que cada variable tenga el suyo) */
static ValorRP evaluar(Interprete *in, const Nodo *valor) {
    switch (valor->clase) {
        case NODO_LITERAL:
            return valor_copiar(&valor->literal);
        case NODO_LLAMADA:
            return llamar(in, valor);
        case NODO_OPERACION: {
            ValorRP izq = evaluar(in, valor->izq);
            ValorRP der = in->hay_error ? NADA : evaluar(in, valor->der);
            ValorRP resultado = NADA;
            char motivo[300];
            if (!in->hay_error && !valor_operar(valor->operador, &izq, &der, &resultado, motivo, sizeof motivo))
                error_ejecucion(in, valor->linea, "%s", motivo);
            valor_liberar(&izq);
            valor_liberar(&der);
            return resultado;
        }
        default: {
            Celda *c = buscar(in, valor->nombre);
            if (!c) {
                error_ejecucion(in, valor->linea, "la variable '%s' no existe", valor->nombre);
                return NADA;
            }
            return valor_copiar(&c->valor);
        }
    }
}

/* Llamada a una chamba: los argumentos se evalúan en el ámbito de quien llama, luego se abre
 * un marco nuevo con los parámetros y se ejecuta el cuerpo hasta el primer 'devolveme' */
static ValorRP llamar(Interprete *in, const Nodo *llamada) {
    const Nodo *chamba = buscar_chamba(in, llamada->nombre);
    if (!chamba) {
        error_ejecucion(in, llamada->linea, "la chamba '%s' no existe", llamada->nombre);
        return NADA;
    }
    if (chamba->num_parametros != llamada->num_parametros) {
        error_ejecucion(in, llamada->linea, "la chamba '%s' recibe %d valor(es), pero se le pasaron %d",
                        chamba->nombre, chamba->num_parametros, llamada->num_parametros);
        return NADA;
    }
    if (in->profundidad >= MAX_PROFUNDIDAD) {
        error_ejecucion(in, llamada->linea, "la chamba '%s' se llamó más de %d veces una dentro de otra; "
                        "revise que no se esté llamando a sí misma sin fin", chamba->nombre, MAX_PROFUNDIDAD);
        return NADA;
    }

    int n = llamada->num_parametros;
    ValorRP *argumentos = xcalloc(n ? (size_t)n : 1, sizeof *argumentos);
    for (int i = 0; i < n && !in->hay_error; i++) argumentos[i] = evaluar(in, llamada->parametros[i]);
    if (in->hay_error) {
        for (int i = 0; i < n; i++) valor_liberar(&argumentos[i]);
        free(argumentos);
        return NADA;
    }

    int marco_anterior = in->marco, globales_anterior = in->globales;
    if (in->profundidad == 0) in->globales = in->cantidad;
    in->marco = in->cantidad;
    in->profundidad++;
    for (int i = 0; i < n; i++) nueva_celda(in, chamba->parametros[i]->nombre)->valor = argumentos[i];
    free(argumentos);

    for (int i = 0; i < chamba->num_sentencias && !in->devolviendo && !in->hay_error; i++)
        interprete_ejecutar_sentencia(in, chamba->sentencias[i]);

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

int interprete_ejecutar_sentencia(Interprete *in, const Nodo *s) {
    switch (s->clase) {
        case NODO_DECLARACION: {
            ValorRP v = s->valor ? evaluar(in, s->valor) : NADA;
            nueva_celda(in, s->nombre)->valor = v;  /* si falló queda en ni maiz, pero la variable existe */
            break;
        }
        case NODO_ASIGNACION: {
            ValorRP v = evaluar(in, s->valor);  /* antes de liberar: 'ponele x a x' */
            Celda *c = buscar(in, s->nombre);
            if (in->hay_error || !c) {
                if (!c) error_ejecucion(in, s->linea, "la variable '%s' no existe", s->nombre);
                valor_liberar(&v);
                break;
            }
            valor_liberar(&c->valor);
            c->valor = v;
            break;
        }
        case NODO_CONSULTA_TIPO: {
            /* El tipo viaja con el valor (unión etiquetada), igual que type() en Python */
            Celda *c = buscar(in, s->nombre);
            if (c)
                in->salida(in->contexto, tipo_rp_nombre(c->valor.tipo));
            else
                error_ejecucion(in, s->linea, "la variable '%s' no existe", s->nombre);
            break;
        }
        case NODO_CONTAME: {
            ValorRP v = evaluar(in, s->valor);
            if (!in->hay_error) {
                char buf[512];
                valor_texto(&v, buf, sizeof buf);
                in->salida(in->contexto, v.tipo == RP_CHISME ? v.dato.chisme : buf);
            }
            valor_liberar(&v);
            break;
        }
        case NODO_DEVOLVEME: {
            ValorRP v = s->valor ? evaluar(in, s->valor) : NADA;
            if (in->hay_error) {
                valor_liberar(&v);
                break;
            }
            in->devuelto = v;
            in->devolviendo = 1;
            break;
        }
        case NODO_CHAMBA:
            registrar_chamba(in, s);  /* el cuerpo se ejecuta hasta que alguien la llame */
            break;
        case NODO_LLAMADA: {
            ValorRP v = llamar(in, s);  /* como sentencia sola, lo que devuelve se descarta */
            valor_liberar(&v);
            break;
        }
        default:
            break;
    }
    return !in->hay_error;
}

int interprete_ejecutar(Interprete *in, const Nodo *programa) {
    for (int i = 0; i < programa->num_sentencias; i++) {
        if (!interprete_ejecutar_sentencia(in, programa->sentencias[i])) return 0;
    }
    return 1;
}

void interprete_liberar(Interprete *in) {
    liberar_hasta(in, 0);
    free(in->memoria);
    free(in->chambas);
    valor_liberar(&in->devuelto);
    in->memoria = NULL;
    in->chambas = NULL;
    in->cantidad = in->capacidad = in->num_chambas = in->capacidad_chambas = 0;
}
