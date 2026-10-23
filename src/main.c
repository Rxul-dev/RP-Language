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

#include "base/util.h"
#include "compilador.h"
#include "fases/interprete.h"
#include "interactivo.h"
#include "reportes.h"

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
            reporte_error(&in.error);
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
