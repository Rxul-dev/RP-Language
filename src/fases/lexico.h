/* Analizador léxico (scanner): convierte el texto fuente en tokens.
 *
 * Expresiones regulares de los tokens:
 *   IDENT       [letra _][letra dígito _]*   (letra incluye á é í ó ú ü ñ)
 *   CABAL       -?[0-9]+
 *   CON_VUELTO  -?[0-9]+\.[0-9]+
 *   CHISME      "[^"\n]*"  |  '[^'\n]*'
 *   símbolos    ( ) [ ] , + - * / == != < > <= >=
 *   comentario  #[^\n]*                       (se descarta)
 *
 * Un '-' pegado a un dígito es el signo de un número (-7) solo si el token
 * anterior no es un valor; después de un valor es una resta: 'x -7' = 'x - 7'.
 */
#ifndef LEXICO_H
#define LEXICO_H

#include "base/errores.h"
#include "fases/token.h"

/* `linea_inicial` permite numerar bien las líneas en el modo interactivo */
void lexico_analizar(const char *fuente, int linea_inicial, ListaTokens *tokens, ListaErrores *errores);

#endif
