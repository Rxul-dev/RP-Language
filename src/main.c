/* Punto de entrada del compilador de RP.
 *
 * Uso:
 *   rp programa.rp        ejecuta y muestra solo la salida del programa
 *   rp programa.rp -f     muestra además cada fase: tokens, AST, tabla AST y tabla de símbolos
 *   rp -i                 modo interactivo, línea por línea
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "compilador.h"
#include "interprete.h"
#include "lexico.h"
#include "reportes.h"
#include "semantico.h"
#include "sintactico.h"
#include "util.h"

static char *leer_archivo(const char *ruta) {
    FILE *f = fopen(ruta, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long largo = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *texto = xmalloc((size_t)largo + 1);
    size_t leidos = fread(texto, 1, (size_t)largo, f);
    texto[leidos] = '\0';
    fclose(f);
    return texto;
}

static int compilar_archivo(const char *ruta, int solo_salida) {
    char *fuente = leer_archivo(ruta);
    if (!fuente) {
        fprintf(stderr, "No se pudo abrir '%s'\n", ruta);
        return 2;
    }

    Resultado r;
    compilar(fuente, &r);

    if (!solo_salida) {
        char titulo[600];
        snprintf(titulo, sizeof titulo, "CÓDIGO FUENTE  (%s)", ruta);
        reporte_titulo(titulo);
        reporte_codigo(fuente);

        reporte_titulo("FASE 1 · ANÁLISIS LÉXICO  →  tabla de tokens");
        reporte_tokens(&r.tokens);
        reporte_errores(&r.errores_lexicos);

        reporte_titulo("FASE 2 · ANÁLISIS SINTÁCTICO  →  AST");
        reporte_arbol_ast(r.programa);
        printf("\nTabla AST:\n");
        reporte_tabla_ast(r.programa);
        reporte_errores(&r.errores_sintacticos);

        reporte_titulo("FASE 3 · ANÁLISIS SEMÁNTICO  →  tabla de símbolos");
        if (r.semantico_ejecutado) {
            reporte_tabla_simbolos(&r.tabla);
            reporte_errores(&r.errores_semanticos);
        } else {
            printf("  (omitido: primero hay que corregir los errores léxicos y sintácticos)\n");
        }
    }

    int codigo = 0;
    if (!resultado_ok(&r)) {
        ListaErrores todos = {0};
        resultado_todos_los_errores(&r, &todos);
        char titulo[100];
        snprintf(titulo, sizeof titulo, "COMPILACIÓN FALLIDA  ·  %d error(es)", todos.cantidad);
        reporte_titulo(titulo);
        reporte_errores(&todos);
        errores_liberar(&todos);
        codigo = 1;
    } else {
        if (!solo_salida) reporte_titulo("FASE 4 · EJECUCIÓN  →  salida del programa");
        Interprete in;
        interprete_iniciar(&in, NULL, NULL);
        if (!interprete_ejecutar(&in, r.programa)) {
            ListaErrores error = {&in.error, 1, 1};
            reporte_errores(&error);
            codigo = 1;
        }
        if (!solo_salida) {
            printf("\nMemoria al terminar:\n");
            reporte_memoria(&in);
        }
        interprete_liberar(&in);
    }

    resultado_liberar(&r);
    free(fuente);
    return codigo;
}

/* 1 si la línea empieza con la palabra `palabra` (ignora espacios al inicio) */
static int empieza_con(const char *linea, const char *palabra) {
    linea += strspn(linea, " \t");
    size_t n = strlen(palabra);
    return strncmp(linea, palabra, n) == 0 && (linea[n] == '\0' || linea[n] == ' ' || linea[n] == '\t' ||
                                               linea[n] == '(' || linea[n] == '#');
}

/* Una chamba ocupa varias líneas: se siguen leyendo hasta 'ya estuvo'. Devuelve las líneas leídas. */
static int leer_chamba(char *buf, size_t n) {
    char linea[1024];
    int lineas = 0;
    while (printf("...> "), fflush(stdout), fgets(linea, sizeof linea, stdin)) {
        lineas++;
        linea[strcspn(linea, "\r\n")] = '\0';
        strncat(buf, "\n", n - strlen(buf) - 1);
        strncat(buf, linea, n - strlen(buf) - 1);
        if (empieza_con(linea, "ya")) break;
    }
    return lineas;
}

static int modo_interactivo(void) {
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
        if (empieza_con(comando, "chamba")) numero_linea += leer_chamba(fuente, sizeof fuente);

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
                    ListaErrores nuevos = {errores_semanticos.datos + antes, errores_semanticos.cantidad - antes, 0};
                    reporte_errores(&nuevos);
                } else if (!interprete_ejecutar_sentencia(&in, programa->sentencias[i])) {
                    ListaErrores error = {&in.error, 1, 1};
                    reporte_errores(&error);
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

static void ayuda(const char *programa) {
    printf("Compilador del lenguaje RP\n\n");
    printf("Uso:\n");
    printf("  %s archivo.rp       ejecuta y muestra solo la salida del programa\n", programa);
    printf("  %s archivo.rp -f    muestra además cada fase: tokens, AST, tabla AST y tabla de símbolos\n", programa);
    printf("  %s -i               modo interactivo línea por línea\n", programa);
}

int main(int argc, char **argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  /* para que se vean bien las tildes y los bordes de las tablas */
    SetConsoleCP(CP_UTF8);
#endif
    const char *archivo = NULL;
    int solo_salida = 1, interactivo = 0;  /* por defecto solo se muestra la salida del programa */

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fases") == 0) solo_salida = 0;
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--solo-salida") == 0) solo_salida = 1;
        else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactivo") == 0) interactivo = 1;
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--ayuda") == 0) { ayuda(argv[0]); return 0; }
        else archivo = argv[i];
    }

    if (interactivo) return modo_interactivo();
    if (!archivo) {
        ayuda(argv[0]);
        return 2;
    }
    return compilar_archivo(archivo, solo_salida);
}
