/* Analizador léxico (scanner): convierte el texto fuente en tokens.
 *
 * Expresiones regulares de los tokens:
 *   IDENT       [letra _][letra dígito _]*   (letra incluye á é í ó ú ü ñ)
 *   CABAL       -?[0-9]+
 *   CON_VUELTO  -?[0-9]+\.[0-9]+
 *   CHISME      "[^"\n]*"  |  '[^'\n]*'
 *   comentario  #[^\n]*                       (se descarta)
 */
#ifndef LEXICO_H
#define LEXICO_H

#include "errores.h"
#include "token.h"

/* `linea_inicial` permite numerar bien las líneas en el modo interactivo */
void lexico_analizar(const char *fuente, int linea_inicial, ListaTokens *tokens, ListaErrores *errores);

#endif
