/* MODO INTERACTIVO (rp -i)
 *
 * Lee una línea, la analiza y la ejecuta enseguida. Las variables y chambas se
 * conservan entre líneas. Un 'si' o una 'chamba' se siguen leyendo hasta su
 * 'ya estuvo'.
 */
#include "interactivo.h"

#include <stdio.h>
#include <string.h>

#include "fases/interprete.h"
#include "fases/lexico.h"
#include "fases/semantico.h"
#include "fases/sintactico.h"
#include "reportes.h"

/* 1 si la línea empieza con la palabra `palabra` (ignora espacios al inicio) */
static int empieza_con(const char *linea, const char *palabra) {
    linea += strspn(linea, " \t");
    size_t n = strlen(palabra);
    return strncmp(linea, palabra, n) == 0 && (linea[n] == '\0' || linea[n] == ' ' || linea[n] == '\t' ||
                                               linea[n] == '(' || linea[n] == '#');
}

static int abre_bloque(const char *linea) {
    return empieza_con(linea, "chamba") || empieza_con(linea, "si");
}

/* Una chamba o un si ocupan varias líneas: se siguen leyendo hasta su 'ya estuvo', contando los
 * bloques que se abren adentro. Devuelve cuántas líneas se leyeron. */
static int leer_bloque(char *buf, size_t n) {
    char linea[1024];
    int lineas = 0, abiertos = 1;
    while (printf("...> "), fflush(stdout), fgets(linea, sizeof linea, stdin)) {
        lineas++;
        linea[strcspn(linea, "\r\n")] = '\0';
        strncat(buf, "\n", n - strlen(buf) - 1);
        strncat(buf, linea, n - strlen(buf) - 1);
        if (abre_bloque(linea)) abiertos++;
        if (empieza_con(linea, "ya") && --abiertos == 0) break;
    }
    return lineas;
}

int modo_interactivo(void) {
    printf("RP interactivo · escriba sentencias de RP línea por línea\n");
    printf("Comandos: :tabla (símbolos), :memoria, :ast (última línea), :salir\n");

    TablaSimbolos tabla;
    ts_iniciar(&tabla);
    ListaErrores errores_semanticos = {0};
    Semantico sem = {&tabla, &errores_semanticos, NULL};
    Interprete in;
    interprete_iniciar(&in, NULL, NULL);
    Nodo *ultimo = NULL;
    /* Los AST se guardan hasta el final: el intérprete apunta a las chambas declaradas en ellos */
    Nodo *anteriores = nodo_nuevo(NODO_PROGRAMA, 0);
    int numero_linea = 0;
    char linea[1024], fuente[16384];

    for (;;) {
        printf("rp> ");
        fflush(stdout);
        if (!fgets(linea, sizeof linea, stdin)) {
            putchar('\n');
            break;
        }
        linea[strcspn(linea, "\r\n")] = '\0';
        const char *comando = linea + strspn(linea, " \t");

        if (strcmp(comando, ":salir") == 0 || strcmp(comando, ":q") == 0) break;
        if (strcmp(comando, ":tabla") == 0) { reporte_tabla_simbolos(&tabla); continue; }
        if (strcmp(comando, ":memoria") == 0) { reporte_memoria(&in); continue; }
        if (strcmp(comando, ":ast") == 0) {
            if (ultimo) reporte_arbol_ast(ultimo); else printf("(todavía no hay AST)\n");
            continue;
        }
        if (*comando == '\0') continue;

        numero_linea++;
        int primera_linea = numero_linea;
        snprintf(fuente, sizeof fuente, "%s", linea);
        if (abre_bloque(comando)) numero_linea += leer_bloque(fuente, sizeof fuente);

        ListaTokens tokens = {0};
        ListaErrores errores = {0};
        lexico_analizar(fuente, primera_linea, &tokens, &errores);
        Nodo *programa = sintactico_analizar(&tokens, &errores);
        tokens_liberar(&tokens);

        if (errores.cantidad > 0) {
            errores_ordenar(&errores);
            reporte_errores(&errores);
            ast_liberar(programa);
        } else {
            for (int i = 0; i < programa->num_sentencias; i++) {
                int antes = errores_semanticos.cantidad;
                semantico_visitar(&sem, programa->sentencias[i]);
                if (errores_semanticos.cantidad > antes) {
                    for (int k = antes; k < errores_semanticos.cantidad; k++) reporte_error(&errores_semanticos.datos[k]);
                } else if (!interprete_ejecutar_sentencia(&in, programa->sentencias[i])) {
                    reporte_error(&in.error);
                    in.hay_error = 0;  /* en modo interactivo se puede seguir escribiendo */
                }
            }
            if (ultimo) nodo_agregar_sentencia(anteriores, ultimo);
            ultimo = programa;
        }
        errores_liberar(&errores);
    }

    ast_liberar(ultimo);
    ast_liberar(anteriores);
    interprete_liberar(&in);
    ts_liberar(&tabla);
    errores_liberar(&errores_semanticos);
    return 0;
}
