# Lenguaje RP — bases de un compilador (en C)

RP es un lenguaje de programación con palabras chapinas. Este proyecto, escrito
en **C estándar (C11) sin librerías externas**, implementa las fases de análisis
de su compilador (léxico, sintáctico y semántico), genera el **AST**, la **tabla
AST** y la **tabla de símbolos**, y además **ejecuta** el programa con un intérprete.

## Cómo compilar y ejecutar

Con `make` (macOS / Linux):

```bash
make                         # genera el ejecutable ./rp
./rp ejemplos/completo.rp    # muestra todas las fases y ejecuta
./rp ejemplos/tipos.rp -s    # solo la salida del programa
./rp -i                      # modo interactivo (comandos :tabla :memoria :ast :salir)
make prueba                  # 47 pruebas de todas las reglas
```

Sin `make` (por ejemplo en Windows con MinGW/gcc):

```bash
gcc -std=c11 -Wall -o rp src/*.c
rp ejemplos\completo.rp
```

## Características del lenguaje

| Característica | En RP |
|---|---|
| Declaración | `fijate que <nombre>` o `fijate que <nombre> es <valor>` |
| Asignación | `ponele <valor> a <nombre>` |
| Consultar tipo | `que onda <nombre>` (como `type()` de Python) |
| Comentarios | `#` hasta el fin de la línea |
| Fin de sentencia | el salto de línea (una sentencia por línea) |
| Tipado | dinámico e inferido: el tipo sale de la forma del valor y puede cambiar |
| Nombres | letra o `_` al inicio; luego letras, dígitos o `_`; se permiten `ñ` y tildes |
| Palabras reservadas | `fijate que es ponele a simon nel ni maiz onda` |

**Tipos de dato:** `cabal` (entero), `con vuelto` (decimal), `chisme` (texto),
`simonel` (`simon`/`nel`), `ni maiz` (sin valor).

## Gramática (EBNF)

```
programa    → { linea } FIN
linea       → [ sentencia ] SALTO
sentencia   → declaracion | asignacion | consulta
declaracion → 'fijate' 'que' IDENT [ 'es' valor ]
asignacion  → 'ponele' valor 'a' IDENT
consulta    → 'que' 'onda' IDENT
valor       → CABAL | CON_VUELTO | CHISME | 'simon' | 'nel' | 'ni' 'maiz' | IDENT
```

Tokens (expresiones regulares):

```
IDENT       [letra _][letra dígito _]*      (y que no sea palabra reservada)
CABAL       -?[0-9]+
CON_VUELTO  -?[0-9]+\.[0-9]+
CHISME      "[^"\n]*"  |  '[^'\n]*'
```

## Arquitectura: fases del compilador

```
código fuente ─► LÉXICO ─► tokens ─► SINTÁCTICO ─► AST ─► SEMÁNTICO ─► AST decorado + tabla de símbolos ─► INTÉRPRETE
```

| Archivo (`src/`) | Fase | Qué hace | Errores que detecta |
|---|---|---|---|
| `lexico.c` | Análisis léxico | Lee carácter por carácter (UTF-8) y agrupa en tokens | `2do`, `.5`, `5.`, `mi-pisto`, `=`, `@`, chisme sin cerrar, número muy grande |
| `sintactico.c` | Análisis sintáctico | Parser de **descenso recursivo**: una función por regla de la gramática; construye el AST | `fijate edad`, `mi pisto`, `fijate que onda`, `ponele 20 edad`, `ponele a edad 20` |
| `semantico.c` | Análisis semántico | Recorre el AST, llena la tabla de símbolos e infiere el tipo de cada valor | Variable declarada dos veces, variable no declarada |
| `tabla_simbolos.c` | Tabla de símbolos | **Tabla hash** (función djb2, encadenamiento): nombre, tipo, ámbito, línea de declaración, asignaciones, usos e historial de tipos | — |
| `ast.c` | AST | Nodos `Programa`, `Declaracion`, `Asignacion`, `ConsultaTipo`, `Literal`, `Variable` | — |
| `interprete.c` | Ejecución | Recorre el AST y guarda los valores en memoria; `que onda` imprime el tipo | — |
| `reportes.c` | Salida | Tabla de tokens, árbol AST, tabla AST, tabla de símbolos | — |
| `token.c`, `valor.c`, `errores.c`, `util.c` | Apoyo | Tipos de token, valores de RP, lista de errores, memoria y UTF-8 | — |
| `compilador.c` | Unión de fases | Corre léxico → sintáctico → semántico | — |
| `main.c` | Programa principal | Lee el archivo, muestra los reportes, modo interactivo | — |

**Tipado dinámico en C:** C es de tipado estático, así que cada valor de RP se
guarda en una **unión etiquetada** (`ValorRP` en `valor.h`): un campo `tipo` y
una `union` con el dato (`long long`, `double`, `char*` o `int`). Así una misma
variable puede guardar un `cabal` y luego un `chisme`, y `que onda` solo lee la
etiqueta.

**Recuperación de errores (modo pánico):** cuando hay un error, se reporta, se
descarta el resto de la línea y el análisis sigue. Por eso un archivo con varios
errores los muestra todos de una vez. Si el léxico ya reportó una línea, el parser
no agrega errores en cascada. El semántico solo corre si no hubo errores léxicos
ni sintácticos, y el programa solo se ejecuta si no hubo ningún error.

**AST decorado:** como RP todavía no tiene condicionales ni ciclos, el semántico
sabe el tipo exacto de cada variable en cada línea y lo anota en el AST (columna
"Tipo" de la tabla AST).

## Guion sugerido para la demostración

1. `./rp ejemplos/completo.rp`: recorrer las 4 fases. Mostrar la tabla de tokens,
   el árbol y la tabla AST, la tabla de símbolos (fíjese en el historial
   `ni maiz → chisme` de `apodo`) y la salida.
2. `./rp ejemplos/tipos.rp -s`: los 5 tipos, nombres con `ñ`/tildes y el cambio de
   tipo `cabal → con vuelto → chisme`.
3. `./rp ejemplos/errores_lexicos.rp`
4. `./rp ejemplos/errores_sintacticos.rp`
5. `./rp ejemplos/errores_semanticos.rp`
   (pasos 3 a 5: errores de cada fase, con línea, columna y sugerencia)
6. `./rp -i`: escribir sentencias en vivo y usar `:tabla` y `:ast`.
