#include "lexico.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

typedef struct {
    const char *fuente;
    size_t pos;
    size_t largo;
    int linea;
    int columna;
    ListaTokens *tokens;
    ListaErrores *errores;
} Lexico;

/* ------------------------------------------------------------ clases de caracteres */

static int es_digito(char c) {
    return c >= '0' && c <= '9';
}

/* Bytes que ocupa la letra al inicio de `s` (0 si no es letra). Acepta las
 * letras ASCII y, en UTF-8, á é í ó ú ü ñ y sus mayúsculas (año, canción). */
static int largo_letra(const char *s) {
    unsigned char c = (unsigned char)s[0];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return 1;
    if (c == 0xC3) {
        static const unsigned char segundo_byte[] = {
            0xA1, 0xA9, 0xAD, 0xB3, 0xBA, 0xBC, 0xB1, /* á é í ó ú ü ñ */
            0x81, 0x89, 0x8D, 0x93, 0x9A, 0x9C, 0x91, /* Á É Í Ó Ú Ü Ñ */
        };
        for (size_t i = 0; i < sizeof segundo_byte; i++) {
            if ((unsigned char)s[1] == segundo_byte[i]) return 2;
        }
    }
    return 0;
}

static int largo_inicio_nombre(const char *s) {
    return s[0] == '_' ? 1 : largo_letra(s);
}

static int largo_parte_nombre(const char *s) {
    return es_digito(s[0]) ? 1 : largo_inicio_nombre(s);
}

/* Largo en bytes de una "palabra": letras, dígitos, '_' y opcionalmente '.' */
static size_t largo_palabra(const char *s, int acepta_punto) {
    size_t n = 0;
    for (;;) {
        int k = largo_parte_nombre(s + n);
        if (k == 0 && acepta_punto && s[n] == '.') k = 1;
        if (k == 0) return n;
        n += (size_t)k;
    }
}

/* Bytes que ocupa un carácter UTF-8 según su primer byte */
static int largo_utf8(const char *s) {
    unsigned char c = (unsigned char)s[0];
    if (c >= 0xF0) return 4;
    if (c >= 0xE0) return 3;
    if (c >= 0xC0) return 2;
    return 1;
}

/* ------------------------------------------------------------ utilidades */

static const char *actual(Lexico *lx) {
    return lx->fuente + lx->pos;
}

static void avanzar(Lexico *lx, size_t bytes) {
    for (size_t i = 0; i < bytes && lx->pos < lx->largo; i++) {
        unsigned char c = (unsigned char)lx->fuente[lx->pos++];
        if (c == '\n') {
            lx->linea++;
            lx->columna = 1;
        } else if ((c & 0xC0) != 0x80) {
            lx->columna++;  /* la columna cuenta caracteres, no bytes */
        }
    }
}

static Token *emitir(Lexico *lx, TipoToken tipo, const char *inicio, size_t n, int linea, int columna) {
    ListaTokens *l = lx->tokens;
    if (l->cantidad == l->capacidad) {
        l->capacidad = l->capacidad ? l->capacidad * 2 : 64;
        l->datos = xrealloc(l->datos, (size_t)l->capacidad * sizeof *l->datos);
    }
    Token *t = &l->datos[l->cantidad++];
    memset(t, 0, sizeof *t);
    t->tipo = tipo;
    t->lexema = xstrndup(inicio, n);
    t->linea = linea;
    t->columna = columna;
    return t;
}

/* Reporta un error y deja un token ERROR para que el parser no agregue errores en cascada */
static void error_lexico(Lexico *lx, const char *inicio, size_t n, int linea, int columna, const char *formato, ...) {
    emitir(lx, TK_ERROR, inicio, n, linea, columna);
    va_list args;
    va_start(args, formato);
    errores_agregarv(lx->errores, FASE_LEXICA, linea, columna, formato, args);
    va_end(args);
}

/* ------------------------------------------------------------ números */

static int es_cabal(const char *s) {
    if (*s == '-') s++;
    if (!es_digito(*s)) return 0;
    while (es_digito(*s)) s++;
    return *s == '\0';
}

static int es_con_vuelto(const char *s) {
    if (*s == '-') s++;
    if (!es_digito(*s)) return 0;
    while (es_digito(*s)) s++;
    if (*s++ != '.' || !es_digito(*s)) return 0;
    while (es_digito(*s)) s++;
    return *s == '\0';
}

static void diagnosticar_numero(Lexico *lx, const char *lexema, size_t n, int linea, int columna) {
    const char *cuerpo = lexema[0] == '-' ? lexema + 1 : lexema;
    int tiene_letra = 0, puntos = 0;
    for (const char *p = cuerpo; *p; p++) {
        if (largo_inicio_nombre(p)) tiene_letra = 1;
        if (*p == '.') puntos++;
    }
    if (tiene_letra && lexema[0] == '-')
        error_lexico(lx, lexema, n, linea, columna, "'%s' no es válido: el '-' no se permite en nombres", lexema);
    else if (tiene_letra)
        error_lexico(lx, lexema, n, linea, columna,
                     "'%s' no es un nombre válido: los nombres no pueden empezar con un dígito", lexema);
    else if (puntos > 1)
        error_lexico(lx, lexema, n, linea, columna, "'%s' no es un número válido: tiene más de un punto", lexema);
    else
        error_lexico(lx, lexema, n, linea, columna,
                     "'%s' no es un número válido: un 'con vuelto' necesita al menos un dígito "
                     "después del punto (ej. 5.0)", lexema);
}

static void numero(Lexico *lx, int linea, int columna) {
    const char *inicio = actual(lx);
    size_t n = inicio[0] == '-' ? 1 : 0;
    /* Se toma la "palabra" completa (2do, 5., 1.2.3) para reportarla en un solo error */
    n += largo_palabra(inicio + n, 1);
    avanzar(lx, n);
    char *lexema = xstrndup(inicio, n);

    if (es_cabal(lexema)) {
        errno = 0;
        long long valor = strtoll(lexema, NULL, 10);
        if (errno == ERANGE)
            error_lexico(lx, lexema, n, linea, columna, "'%s' es demasiado grande para un cabal", lexema);
        else
            emitir(lx, TK_CABAL, lexema, n, linea, columna)->entero = valor;
    } else if (es_con_vuelto(lexema)) {
        emitir(lx, TK_CON_VUELTO, lexema, n, linea, columna)->decimal = strtod(lexema, NULL);
    } else {
        diagnosticar_numero(lx, lexema, n, linea, columna);
    }
    free(lexema);
}

/* ------------------------------------------------------------ otros tokens */

static void palabra(Lexico *lx, int linea, int columna) {
    const char *inicio = actual(lx);
    size_t n = largo_palabra(inicio, 0);
    avanzar(lx, n);
    Token *t = emitir(lx, TK_IDENT, inicio, n, linea, columna);
    t->tipo = buscar_palabra_reservada(t->lexema);
}

static void chisme(Lexico *lx, int linea, int columna) {
    const char *inicio = actual(lx);
    char comilla = inicio[0];
    size_t n = 1;
    while (inicio[n] != '\0' && inicio[n] != '\n' && inicio[n] != comilla) n++;
    if (inicio[n] == comilla) {
        avanzar(lx, n + 1);
        emitir(lx, TK_CHISME, inicio, n + 1, linea, columna);
    } else {
        avanzar(lx, n);
        error_lexico(lx, inicio, n, linea, columna,
                     "chisme sin cerrar: falta la comilla de cierre %c antes del fin de la línea", comilla);
    }
}

static void simbolo(Lexico *lx, int linea, int columna) {
    const char *inicio = actual(lx);
    size_t n = (size_t)largo_utf8(inicio);
    if (n > lx->largo - lx->pos) n = lx->largo - lx->pos;
    avanzar(lx, n);
    switch (inicio[0]) {
        case '=':
            error_lexico(lx, inicio, n, linea, columna,
                         "RP no usa '='; para declarar use 'fijate que <nombre> es <valor>' "
                         "y para asignar 'ponele <valor> a <nombre>'");
            break;
        case '-':
            error_lexico(lx, inicio, n, linea, columna,
                         "el signo '-' solo va pegado al inicio de un número (ej. -7); no se permite en nombres");
            break;
        case '.':
            error_lexico(lx, inicio, n, linea, columna,
                         "punto suelto: un 'con vuelto' lleva dígitos a ambos lados del punto (ej. 2.5)");
            break;
        default:
            error_lexico(lx, inicio, n, linea, columna, "símbolo '%.*s' no permitido en RP", (int)n, inicio);
    }
}

/* ------------------------------------------------------------ análisis */

void lexico_analizar(const char *fuente, int linea_inicial, ListaTokens *tokens, ListaErrores *errores) {
    Lexico lx = {fuente, 0, strlen(fuente), linea_inicial, 1, tokens, errores};

    /* Los archivos guardados con el Bloc de notas de Windows pueden empezar con un BOM UTF-8 */
    if (strncmp(fuente, "\xEF\xBB\xBF", 3) == 0) lx.pos = 3;

    while (lx.pos < lx.largo) {
        const char *s = actual(&lx);
        int linea = lx.linea, columna = lx.columna;

        if (*s == ' ' || *s == '\t' || *s == '\r') {
            avanzar(&lx, 1);
        } else if (*s == '\n') {
            emitir(&lx, TK_SALTO, "\\n", 2, linea, columna);
            avanzar(&lx, 1);
        } else if (*s == '#') {
            while (lx.pos < lx.largo && *actual(&lx) != '\n') avanzar(&lx, 1);
        } else if (*s == '"' || *s == '\'') {
            chisme(&lx, linea, columna);
        } else if (es_digito(*s) || (*s == '-' && es_digito(s[1]))) {
            numero(&lx, linea, columna);
        } else if (*s == '.' && es_digito(s[1])) {
            size_t n = largo_palabra(s, 1);
            avanzar(&lx, n);
            error_lexico(&lx, s, n, linea, columna,
                         "'%.*s' no es un número válido: un 'con vuelto' necesita al menos un dígito "
                         "antes del punto (ej. 0.5)", (int)n, s);
        } else if (largo_inicio_nombre(s)) {
            palabra(&lx, linea, columna);
        } else {
            simbolo(&lx, linea, columna);
        }
    }

    if (tokens->cantidad == 0 || tokens->datos[tokens->cantidad - 1].tipo != TK_SALTO)
        emitir(&lx, TK_SALTO, "\\n", 2, lx.linea, lx.columna);
    emitir(&lx, TK_FIN, "", 0, lx.linea, lx.columna);
}
