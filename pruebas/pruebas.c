/* Pruebas de las reglas de RP. Ejecutar con: make prueba */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compilador.h"
#include "interprete.h"

static int total = 0, fallos = 0;

#define VERIFICAR(condicion, ...)                                   \
    do {                                                            \
        total++;                                                    \
        if (!(condicion)) {                                         \
            fallos++;                                               \
            printf("  ✘ FALLO (%s:%d): ", __FILE__, __LINE__);      \
            printf(__VA_ARGS__);                                    \
            printf("\n");                                           \
        }                                                           \
    } while (0)

/* Junta en `buf` todo lo que imprime el programa, separado por '|' */
static void capturar(void *contexto, const char *texto) {
    char *buf = contexto;
    if (buf[0]) strcat(buf, "|");
    strcat(buf, texto);
}

/* Compila y ejecuta; devuelve 1 si compiló sin errores */
static int ejecutar(const char *fuente, char *salida) {
    Resultado r;
    compilar(fuente, &r);
    int ok = resultado_ok(&r);
    salida[0] = '\0';
    if (ok) {
        Interprete in;
        interprete_iniciar(&in, capturar, salida);
        interprete_ejecutar(&in, r.programa);
        interprete_liberar(&in);
    }
    resultado_liberar(&r);
    return ok;
}

/* Fase del primer error, o -1 si compiló sin errores */
static int fase_del_error(const char *fuente) {
    Resultado r;
    compilar(fuente, &r);
    ListaErrores todos = {0};
    resultado_todos_los_errores(&r, &todos);
    int fase = todos.cantidad ? (int)todos.datos[0].fase : -1;
    errores_liberar(&todos);
    resultado_liberar(&r);
    return fase;
}

static void verificar_salida(const char *fuente, const char *esperada) {
    char salida[1024];
    int ok = ejecutar(fuente, salida);
    VERIFICAR(ok && strcmp(salida, esperada) == 0, "'%s' → se esperaba '%s', salió '%s'", fuente, esperada, salida);
}

static void verificar_error(const char *fuente, Fase fase) {
    int obtenida = fase_del_error(fuente);
    VERIFICAR(obtenida == (int)fase, "'%s' → se esperaba error %s", fuente, fase_nombre(fase));
}

static void pruebas_declaracion(void) {
    const char *validas[] = {
        "fijate que edad", "fijate que nombre es \"Ana\"", "fijate que año es 2026",
        "fijate que _x1 es nel", "fijate que canción es 'hola'",
    };
    for (size_t i = 0; i < sizeof validas / sizeof *validas; i++)
        VERIFICAR(fase_del_error(validas[i]) == -1, "'%s' debería ser válida", validas[i]);

    verificar_error("fijate que 2do es 5", FASE_LEXICA);
    verificar_error("fijate que mi pisto es 100", FASE_SINTACTICA);
    verificar_error("fijate que onda es 3", FASE_SINTACTICA);
    verificar_error("edad es 20", FASE_SINTACTICA);
    verificar_error("fijate edad es 20", FASE_SINTACTICA);
    verificar_error("fijate que mi-pisto es 1", FASE_LEXICA);
    verificar_error("fijate que x@ es 1", FASE_LEXICA);
    verificar_error("fijate que x\nfijate que x", FASE_SEMANTICA);

    verificar_salida("fijate que edad\nque onda edad", "ni maiz");
}

static void pruebas_asignacion(void) {
    const char *base = "fijate que edad es 19\nfijate que nombre es \"Ana\"\nfijate que copia\n";
    const char *validas[] = {"ponele 20 a edad", "ponele \"Luis\" a nombre", "ponele edad a copia", "ponele 8.5 a edad"};
    char fuente[512];
    for (size_t i = 0; i < sizeof validas / sizeof *validas; i++) {
        snprintf(fuente, sizeof fuente, "%s%s", base, validas[i]);
        VERIFICAR(fase_del_error(fuente) == -1, "'%s' debería ser válida", validas[i]);
    }

    verificar_error("fijate que edad es 19\nponele 500 a sueldo", FASE_SEMANTICA);
    verificar_error("fijate que edad es 19\nponele a edad 20", FASE_SINTACTICA);
    verificar_error("fijate que edad es 19\nponele 20 edad", FASE_SINTACTICA);
    verificar_error("fijate que edad es 19\nedad = 20", FASE_LEXICA);
    verificar_error("fijate que edad es 19\nponele bono a edad", FASE_SEMANTICA);

    /* Tipado dinámico: la variable cambia de tipo */
    verificar_salida("fijate que edad es 19\nponele \"veinte\" a edad\nque onda edad", "chisme");
    verificar_salida("fijate que a1 es simon\nfijate que b\nponele a1 a b\nque onda b", "simonel");
    verificar_salida("fijate que x es \"hola\"\nponele x a x\nque onda x", "chisme");

    Resultado r;
    compilar("fijate que edad es 19\nponele 8.5 a edad\nponele \"x\" a edad", &r);
    Simbolo *s = ts_buscar(&r.tabla, "edad");
    VERIFICAR(s && s->historial.cantidad == 3 && s->historial.datos[0] == RP_CABAL &&
                  s->historial.datos[1] == RP_CON_VUELTO && s->historial.datos[2] == RP_CHISME,
              "historial de tipos de 'edad'");
    resultado_liberar(&r);
}

static void pruebas_tipos(void) {
    const char *valores[][2] = {
        {"150", "cabal"}, {"-3", "cabal"}, {"2.5", "con vuelto"}, {"-0.5", "con vuelto"},
        {"\"Ana\"", "chisme"}, {"'Ana'", "chisme"}, {"\"\"", "chisme"}, {"\"simon\"", "chisme"},
        {"simon", "simonel"}, {"nel", "simonel"}, {"ni maiz", "ni maiz"},
    };
    char fuente[256];
    for (size_t i = 0; i < sizeof valores / sizeof *valores; i++) {
        snprintf(fuente, sizeof fuente, "fijate que v es %s\nque onda v", valores[i][0]);
        verificar_salida(fuente, valores[i][1]);
    }

    verificar_error("fijate que v es .5", FASE_LEXICA);
    verificar_error("fijate que v es 5.", FASE_LEXICA);
    verificar_error("fijate que v es 1.2.3", FASE_LEXICA);
    verificar_error("fijate que v es - 5", FASE_LEXICA);
    verificar_error("fijate que v es \"hola", FASE_LEXICA);
    verificar_error("fijate que v es 99999999999999999999", FASE_LEXICA);
    verificar_error("fijate que v es ni", FASE_SINTACTICA);
}

static void prueba_ejemplo_completo(void) {
    FILE *f = fopen("ejemplos/completo.rp", "rb");
    VERIFICAR(f != NULL, "no se encontró ejemplos/completo.rp (ejecute desde la carpeta del proyecto)");
    if (!f) return;
    char fuente[4096];
    size_t n = fread(fuente, 1, sizeof fuente - 1, f);
    fuente[n] = '\0';
    fclose(f);
    verificar_salida(fuente, "cabal|chisme");
}

int main(void) {
    pruebas_declaracion();
    pruebas_asignacion();
    pruebas_tipos();
    prueba_ejemplo_completo();
    printf("%d de %d pruebas pasaron\n", total - fallos, total);
    return fallos ? 1 : 0;
}
