#include "reportes.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/* ------------------------------------------------------------ tabla genérica */

typedef struct {
    int columnas;
    const char *const *encabezados;
    char **celdas;  /* filas * columnas, por filas */
    int filas;
    int capacidad;
} Tabla;

static void tabla_iniciar(Tabla *t, int columnas, const char *const *encabezados) {
    memset(t, 0, sizeof *t);
    t->columnas = columnas;
    t->encabezados = encabezados;
}

static int tabla_fila(Tabla *t) {
    if (t->filas == t->capacidad) {
        t->capacidad = t->capacidad ? t->capacidad * 2 : 16;
        t->celdas = xrealloc(t->celdas, (size_t)(t->capacidad * t->columnas) * sizeof(char *));
    }
    for (int c = 0; c < t->columnas; c++) t->celdas[t->filas * t->columnas + c] = NULL;
    return t->filas++;
}

static void tabla_poner(Tabla *t, int fila, int columna, const char *formato, ...) {
    char buf[1024];
    va_list args;
    va_start(args, formato);
    vsnprintf(buf, sizeof buf, formato, args);
    va_end(args);
    char **celda = &t->celdas[fila * t->columnas + columna];
    free(*celda);
    *celda = xstrdup(buf);
}

static const char *tabla_celda(const Tabla *t, int fila, int columna) {
    const char *c = t->celdas[fila * t->columnas + columna];
    return c ? c : "";
}

static void repetir(const char *s, size_t veces) {
    for (size_t i = 0; i < veces; i++) fputs(s, stdout);
}

static void borde(const size_t *anchos, int columnas, const char *izq, const char *medio, const char *der) {
    fputs(izq, stdout);
    for (int c = 0; c < columnas; c++) {
        repetir("─", anchos[c] + 2);
        fputs(c + 1 < columnas ? medio : der, stdout);
    }
    putchar('\n');
}

static void fila_texto(const size_t *anchos, int columnas, const char *const *textos) {
    fputs("│", stdout);
    for (int c = 0; c < columnas; c++) {
        printf(" %s", textos[c]);
        repetir(" ", anchos[c] - utf8_ancho(textos[c]) + 1);  /* relleno según caracteres, no bytes */
        fputs("│", stdout);
    }
    putchar('\n');
}

static void tabla_imprimir(const Tabla *t) {
    size_t *anchos = xcalloc((size_t)t->columnas, sizeof *anchos);
    const char **textos = xcalloc((size_t)t->columnas, sizeof *textos);
    for (int c = 0; c < t->columnas; c++) {
        anchos[c] = utf8_ancho(t->encabezados[c]);
        for (int f = 0; f < t->filas; f++) {
            size_t a = utf8_ancho(tabla_celda(t, f, c));
            if (a > anchos[c]) anchos[c] = a;
        }
    }
    borde(anchos, t->columnas, "┌", "┬", "┐");
    fila_texto(anchos, t->columnas, t->encabezados);
    borde(anchos, t->columnas, "├", "┼", "┤");
    for (int f = 0; f < t->filas; f++) {
        for (int c = 0; c < t->columnas; c++) textos[c] = tabla_celda(t, f, c);
        fila_texto(anchos, t->columnas, textos);
    }
    borde(anchos, t->columnas, "└", "┴", "┘");
    free(textos);
    free(anchos);
}

static void tabla_liberar(Tabla *t) {
    for (int i = 0; i < t->filas * t->columnas; i++) free(t->celdas[i]);
    free(t->celdas);
}

/* ------------------------------------------------------------ reportes */

void reporte_titulo(const char *texto) {
    printf("\n");
    repetir("═", 70);
    printf("\n  %s\n", texto);
    repetir("═", 70);
    printf("\n");
}

void reporte_codigo(const char *fuente) {
    int linea = 1;
    printf("%3d │ ", linea);
    for (const char *p = fuente; *p; p++) {
        if (*p == '\n') {
            if (p[1] == '\0') break;
            printf("\n%3d │ ", ++linea);
        } else if (*p != '\r') {
            putchar(*p);
        }
    }
    putchar('\n');
}

void reporte_tokens(const ListaTokens *tokens) {
    static const char *const encabezados[] = {"#", "Lín", "Col", "Token", "Categoría", "Lexema"};
    Tabla t;
    tabla_iniciar(&t, 6, encabezados);
    for (int i = 0; i < tokens->cantidad; i++) {
        const Token *tk = &tokens->datos[i];
        int f = tabla_fila(&t);
        tabla_poner(&t, f, 0, "%d", i + 1);
        tabla_poner(&t, f, 1, "%d", tk->linea);
        tabla_poner(&t, f, 2, "%d", tk->columna);
        tabla_poner(&t, f, 3, "%s", token_nombre(tk->tipo));
        tabla_poner(&t, f, 4, "%s", token_categoria(tk->tipo));
        tabla_poner(&t, f, 5, "%s", tk->lexema);
    }
    tabla_imprimir(&t);
    tabla_liberar(&t);
}

static void visitar_arbol(const Nodo *nodo, const char *prefijo, int es_ultimo, int es_raiz) {
    char atributos[512], prefijo_hijos[512];
    nodo_atributos(nodo, atributos, sizeof atributos);
    if (es_raiz) {
        printf("%s (%s)", nodo_etiqueta(nodo), atributos);
        prefijo_hijos[0] = '\0';
    } else {
        printf("%s%s%s (%s)", prefijo, es_ultimo ? "└── " : "├── ", nodo_etiqueta(nodo), atributos);
        snprintf(prefijo_hijos, sizeof prefijo_hijos, "%s%s", prefijo, es_ultimo ? "    " : "│   ");
    }
    if (nodo->tiene_tipo) printf(" : %s", tipo_rp_nombre(nodo->tipo));
    putchar('\n');

    int hijos = nodo_num_hijos(nodo);
    for (int i = 0; i < hijos; i++) visitar_arbol(nodo_hijo(nodo, i), prefijo_hijos, i == hijos - 1, 0);
}

void reporte_arbol_ast(const Nodo *programa) {
    visitar_arbol(programa, "", 1, 1);
}

/* Agrega a la tabla el nodo y sus hijos (preorden); devuelve el id asignado al nodo */
static int visitar_tabla_ast(Tabla *t, const Nodo *nodo, int padre) {
    char atributos[512];
    int f = tabla_fila(t);
    int id = f + 1;
    nodo_atributos(nodo, atributos, sizeof atributos);
    tabla_poner(t, f, 0, "%d", id);
    tabla_poner(t, f, 1, "%s", nodo_etiqueta(nodo));
    tabla_poner(t, f, 2, "%d", nodo->linea);
    tabla_poner(t, f, 3, "%s", atributos);
    tabla_poner(t, f, 4, "%s", nodo->tiene_tipo ? tipo_rp_nombre(nodo->tipo) : "-");
    if (padre) tabla_poner(t, f, 5, "%d", padre); else tabla_poner(t, f, 5, "-");

    int hijos = nodo_num_hijos(nodo);
    size_t capacidad = (size_t)hijos * 12 + 2;
    char *ids = xmalloc(capacidad);
    strcpy(ids, hijos ? "" : "-");
    for (int i = 0; i < hijos; i++) {
        int id_hijo = visitar_tabla_ast(t, nodo_hijo(nodo, i), id);
        size_t usado = strlen(ids);
        snprintf(ids + usado, capacidad - usado, "%s%d", i ? ", " : "", id_hijo);
    }
    tabla_poner(t, f, 6, "%s", ids);
    free(ids);
    return id;
}

void reporte_tabla_ast(const Nodo *programa) {
    static const char *const encabezados[] = {"Id", "Nodo", "Lín", "Atributos", "Tipo", "Padre", "Hijos"};
    Tabla t;
    tabla_iniciar(&t, 7, encabezados);
    visitar_tabla_ast(&t, programa, 0);
    tabla_imprimir(&t);
    tabla_liberar(&t);
}

static void unir_lineas(const ListaEnteros *l, char *buf, size_t n) {
    buf[0] = '\0';
    if (l->cantidad == 0) snprintf(buf, n, "-");
    for (int i = 0; i < l->cantidad; i++) {
        size_t usado = strlen(buf);
        snprintf(buf + usado, n - usado, "%s%d", i ? ", " : "", l->datos[i]);
    }
}

void reporte_tabla_simbolos(const TablaSimbolos *tabla) {
    static const char *const encabezados[] = {
        "Nombre", "Clase", "Tipo actual", "Ámbito", "Declarada", "Asignada en", "Usada en", "Historial de tipos",
    };
    if (tabla->cantidad == 0) {
        printf("(vacía)\n");
        return;
    }
    Tabla t;
    tabla_iniciar(&t, 8, encabezados);
    char buf[1024];
    for (int i = 0; i < tabla->cantidad; i++) {
        const Simbolo *s = tabla->orden[i];
        int f = tabla_fila(&t);
        tabla_poner(&t, f, 0, "%s", s->nombre);
        if (s->clase == SIM_CHAMBA)
            tabla_poner(&t, f, 1, "chamba (%d parámetros)", s->num_parametros);
        else
            tabla_poner(&t, f, 1, "%s", clase_simbolo_nombre(s->clase));
        tabla_poner(&t, f, 2, "%s", tipo_rp_nombre(s->tipo));
        tabla_poner(&t, f, 3, "%s", s->ambito);
        tabla_poner(&t, f, 4, "%d", s->linea_declaracion);
        unir_lineas(&s->asignaciones, buf, sizeof buf);
        tabla_poner(&t, f, 5, "%s", buf);
        unir_lineas(&s->usos, buf, sizeof buf);
        tabla_poner(&t, f, 6, "%s", buf);
        buf[0] = '\0';
        for (int k = 0; k < s->historial.cantidad; k++) {
            size_t usado = strlen(buf);
            snprintf(buf + usado, sizeof buf - usado, "%s%s", k ? " → " : "",
                     tipo_rp_nombre((TipoRP)s->historial.datos[k]));
        }
        tabla_poner(&t, f, 7, "%s", buf);
    }
    tabla_imprimir(&t);
    tabla_liberar(&t);
}

void reporte_memoria(const Interprete *in) {
    static const char *const encabezados[] = {"Variable", "Valor", "Tipo"};
    if (in->cantidad == 0) {
        printf("(vacía)\n");
        return;
    }
    Tabla t;
    tabla_iniciar(&t, 3, encabezados);
    char buf[1024];
    for (int i = 0; i < in->cantidad; i++) {
        int f = tabla_fila(&t);
        valor_formatear(&in->memoria[i].valor, buf, sizeof buf);
        tabla_poner(&t, f, 0, "%s", in->memoria[i].nombre);
        tabla_poner(&t, f, 1, "%s", buf);
        tabla_poner(&t, f, 2, "%s", tipo_rp_nombre(in->memoria[i].valor.tipo));
    }
    tabla_imprimir(&t);
    tabla_liberar(&t);
}

void reporte_errores(const ListaErrores *errores) {
    char buf[800];
    if (errores->cantidad == 0) {
        printf("  ✔ Sin errores\n");
        return;
    }
    for (int i = 0; i < errores->cantidad; i++) {
        error_formatear(&errores->datos[i], buf, sizeof buf);
        printf("  ✘ %s\n", buf);
    }
}
