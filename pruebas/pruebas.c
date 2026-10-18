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

/* Compila y ejecuta; devuelve 1 si compiló sin errores. Un error de ejecución se agrega a la
 * salida como "ERROR: <mensaje>". */
static int ejecutar(const char *fuente, char *salida) {
    Resultado r;
    compilar(fuente, &r);
    int ok = resultado_ok(&r);
    salida[0] = '\0';
    if (ok) {
        Interprete in;
        interprete_iniciar(&in, capturar, salida);
        if (!interprete_ejecutar(&in, r.programa)) {
            char error[700];
            snprintf(error, sizeof error, "ERROR: %s", in.error.mensaje);
            capturar(salida, error);
        }
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
    verificar_error("fijate que v es - 5", FASE_SINTACTICA);  /* '-' suelto: el negativo va pegado */
    verificar_error("fijate que v es \"hola", FASE_LEXICA);
    verificar_error("fijate que v es 99999999999999999999", FASE_LEXICA);
    verificar_error("fijate que v es ni", FASE_SINTACTICA);
}

static void pruebas_expresiones(void) {
    verificar_salida("contame 2 + 3 * 4", "14");
    verificar_salida("contame (2 + 3) * 4", "20");
    verificar_salida("contame 10 - 2 - 3", "5");
    verificar_salida("contame 7 / 2", "3");
    verificar_salida("contame 7 / 2.0", "3.5");
    verificar_salida("contame 5-3", "2");
    verificar_salida("fijate que x es 5\ncontame x -1", "4");
    verificar_salida("contame -2 * -3", "6");
    verificar_salida("contame \"Edad: \" + 19", "Edad: 19");
    verificar_salida("contame \"hola\"", "hola");
    verificar_salida("contame simon", "simon");
    verificar_salida("fijate que x es 2.5 * 2\nque onda x", "con vuelto");
    verificar_salida("fijate que x es 3\nponele x * 2 + 1 a x\ncontame x", "7");

    verificar_error("contame \"a\" - 1", FASE_SEMANTICA);
    verificar_error("contame simon + 1", FASE_SEMANTICA);
    verificar_error("contame (1 + 2", FASE_SINTACTICA);
    verificar_error("contame 1 +", FASE_SINTACTICA);
    verificar_error("fijate que m es 1\nfijate que n es 2\ncontame m-n", FASE_LEXICA);
    verificar_salida("contame 1 / 0", "ERROR: división entre cero");
}

static void pruebas_chambas(void) {
    const char *profe =
        "fijate que x es 30\n"
        "fijate que y es 20\n"
        "chamba sumar(x, y)\n"
        "    devolveme x + y\n"
        "ya estuvo\n"
        "chamba avisar(mensaje)\n"
        "    contame mensaje\n"
        "    devolveme\n"
        "ya estuvo\n"
        "fijate que resultado es sumar(x, y) * 2 / 4 - 3\n"
        "avisar(\"resultado calculado\")\n"
        "contame resultado\n";
    verificar_salida(profe, "resultado calculado|22");

    /* Sin 'devolveme' devuelve ni maiz; lo que sigue a 'devolveme' no se ejecuta */
    verificar_salida("chamba f()\n  contame 1\nya estuvo\nfijate que v es f()\nque onda v", "1|ni maiz");
    verificar_salida("chamba f()\n  devolveme 1\n  contame 99\nya estuvo\ncontame f()", "1");
    /* Las variables locales no se ven afuera ni se mezclan con las globales */
    verificar_salida("fijate que t es \"global\"\nchamba f()\n  fijate que t es 5\n  devolveme t\nya estuvo\n"
                     "contame f()\ncontame t", "5|global");
    /* Una chamba lee las globales y puede llamar a otra */
    verificar_salida("fijate que base es 10\nchamba doble(n)\n  devolveme n * 2\nya estuvo\n"
                     "chamba f(n)\n  devolveme doble(n) + base\nya estuvo\ncontame f(4)", "18");
    /* El tipo de un parámetro se revisa al ejecutar */
    verificar_salida("chamba resta(m, n)\n  devolveme m - n\nya estuvo\ncontame resta(\"x\", 1)",
                     "ERROR: no se puede usar '-' entre un chisme y un cabal; "
                     "esta operación solo funciona con números (cabal o con vuelto)");
    /* Recursión sin fin */
    char salida[1024];
    ejecutar("chamba f()\n  devolveme f()\nya estuvo\ncontame f()", salida);
    VERIFICAR(strncmp(salida, "ERROR: la chamba 'f' se llamó más de", 36) == 0, "recursión sin fin: '%s'", salida);

    /* Errores sintácticos */
    verificar_error("chamba f(\n  devolveme 1\nya estuvo", FASE_SINTACTICA);
    verificar_error("chamba f()\n  devolveme 1", FASE_SINTACTICA);
    verificar_error("chamba f() devolveme 1\nya estuvo", FASE_SINTACTICA);
    verificar_error("chamba f()\n  chamba g()\n  ya estuvo\nya estuvo", FASE_SINTACTICA);
    verificar_error("ya estuvo", FASE_SINTACTICA);
    verificar_error("chamba f(a)\nya estuvo", FASE_SINTACTICA);  /* 'a' es palabra reservada */
    verificar_error("chamba que()\nya estuvo", FASE_SINTACTICA);
    verificar_error("fijate que contame es 1", FASE_SINTACTICA);
    verificar_error("chamba f()\nya\n", FASE_SINTACTICA);

    /* Errores semánticos */
    verificar_error("devolveme 1", FASE_SEMANTICA);
    verificar_error("chamba f(p, p)\nya estuvo", FASE_SEMANTICA);
    verificar_error("chamba f()\nya estuvo\nchamba f()\nya estuvo", FASE_SEMANTICA);
    verificar_error("chamba f(p)\nya estuvo\nf(1, 2)", FASE_SEMANTICA);
    verificar_error("g()", FASE_SEMANTICA);
    verificar_error("fijate que g es 1\ng()", FASE_SEMANTICA);
    verificar_error("chamba f()\nya estuvo\ncontame f", FASE_SEMANTICA);
    verificar_error("chamba f()\nya estuvo\nfijate que f es 1", FASE_SEMANTICA);
    verificar_error("chamba f(p)\n  fijate que p es 1\nya estuvo", FASE_SEMANTICA);
    verificar_error("chamba f()\n  fijate que local es 1\nya estuvo\ncontame local", FASE_SEMANTICA);
    verificar_error("fijate que g es 1\nchamba f()\n  ponele 2 a g\nya estuvo", FASE_SEMANTICA);

    /* Ámbitos en la tabla de símbolos */
    Resultado r;
    compilar("fijate que x es 1\nchamba f(x)\n  fijate que y es x\nya estuvo", &r);
    int encontrados = 0;
    for (int i = 0; i < r.tabla.cantidad; i++) {
        const Simbolo *s = r.tabla.orden[i];
        if (strcmp(s->nombre, "f") == 0 && s->clase == SIM_CHAMBA && s->num_parametros == 1) encontrados++;
        if (strcmp(s->nombre, "x") == 0 && s->clase == SIM_PARAMETRO && strcmp(s->ambito, "chamba f") == 0)
            encontrados++;
        if (strcmp(s->nombre, "y") == 0 && strcmp(s->ambito, "chamba f") == 0) encontrados++;
    }
    VERIFICAR(resultado_ok(&r) && encontrados == 3, "ámbitos de la chamba 'f' en la tabla de símbolos");
    VERIFICAR(ts_buscar(&r.tabla, "y") == NULL, "'y' no debería verse desde el ámbito global");
    resultado_liberar(&r);
}

static void pruebas_si(void) {
    /* Comparaciones */
    verificar_salida("contame 3 > 2", "simon");
    verificar_salida("contame 3 <= 2", "nel");
    verificar_salida("contame 2 == 2.0", "simon");
    verificar_salida("contame \"hola\" == \"hola\"", "simon");
    verificar_salida("contame \"hola\" != \"chao\"", "simon");
    verificar_salida("contame 1 == \"1\"", "nel");
    verificar_salida("fijate que x\ncontame x == ni maiz", "simon");
    verificar_salida("contame 1 + 1 == 2", "simon");
    verificar_salida("contame 3 > -5", "simon");
    verificar_salida("fijate que mayor es 20 >= 18\nque onda mayor", "simonel");

    /* El cuerpo solo se ejecuta si la condición es simon */
    verificar_salida("si 5 > 3\n  contame \"sí\"\nya estuvo\ncontame \"fin\"", "sí|fin");
    verificar_salida("si 1 > 2\n  contame \"no\"\nya estuvo\ncontame \"fin\"", "fin");
    verificar_salida("si simon\n  si nel\n    contame 1\n  ya estuvo\n  contame 2\nya estuvo", "2");
    /* 'devolveme' dentro de un si termina la chamba */
    verificar_salida("chamba factorial(n)\n  si n <= 1\n    devolveme 1\n  ya estuvo\n"
                     "  devolveme n * factorial(n - 1)\nya estuvo\ncontame factorial(5)", "120");
    /* Una variable que cambia de tipo dentro del si queda dinámica: no hay error semántico falso */
    verificar_salida("fijate que x es 1\nsi nel\n  ponele \"a\" a x\nya estuvo\ncontame x + 1", "2");
    /* Una variable declarada en un si que no se cumplió no existe */
    verificar_salida("si nel\n  fijate que v es 1\nya estuvo\ncontame v",
                     "ERROR: la variable 'v' no existe (¿se declaró dentro de un 'si' que no se cumplió?)");
    /* La condición dinámica se revisa al ejecutar */
    verificar_salida("chamba f(c)\n  si c\n    contame 1\n  ya estuvo\nya estuvo\nf(5)",
                     "ERROR: la condición del 'si' debe dar simon o nel, pero dio un valor de tipo cabal");

    /* Errores */
    verificar_error("fijate que x = 5", FASE_LEXICA);
    verificar_error("contame 1 ! 2", FASE_LEXICA);
    verificar_error("si 1 >\nya estuvo", FASE_SINTACTICA);
    verificar_error("si 1 > 2\n  contame 1", FASE_SINTACTICA);
    verificar_error("si 1 > 2 contame 1\nya estuvo", FASE_SINTACTICA);
    verificar_error("contame 1 < 2 < 3", FASE_SINTACTICA);
    verificar_error("si simon\n  chamba f()\n  ya estuvo\nya estuvo", FASE_SINTACTICA);
    verificar_error("fijate que si es 1", FASE_SINTACTICA);
    verificar_error("si 5\nya estuvo", FASE_SEMANTICA);
    verificar_error("contame \"a\" < 1", FASE_SEMANTICA);
    verificar_error("si simon\n  devolveme 1\nya estuvo", FASE_SEMANTICA);
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
    pruebas_expresiones();
    pruebas_chambas();
    pruebas_si();
    prueba_ejemplo_completo();
    printf("%d de %d pruebas pasaron\n", total - fallos, total);
    return fallos ? 1 : 0;
}
