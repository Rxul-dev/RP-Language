# Lenguaje RP

<p align="center"><img src="logo/logo.png" alt="Logo del lenguaje RP" width="140"></p>

RP es un lenguaje de programación con instrucciones en español coloquial de
Guatemala. Este repositorio contiene su compilador, escrito en C sin
dependencias externas.

```
fijate que nombre es "Ana"

chamba saludar(persona)
    devolveme "Hola, " + persona
ya estuvo

contame saludar(nombre)        # Hola, Ana
```

> Proyecto académico del curso de Compiladores: análisis léxico, sintáctico y
> semántico, AST, tabla de símbolos e intérprete.

## Instalación

Requiere un compilador de C.

| Sistema | Instalar compilador de C | Compilar RP |
|---|---|---|
| macOS | `xcode-select --install` | `make` |
| Windows | `winget install BrechtSanders.WinLibs.POSIX.UCRT` | `.\compilar.bat` |

Para comprobar la instalación:

```bash
./rp ejemplos/chambas.rp       # Windows: .\rp.exe ejemplos\chambas.rp
```

## Uso

| Comando | Descripción |
|---|---|
| `./rp archivo.rp` | Ejecuta el programa |
| `./rp archivo.rp -f` | Muestra además cada fase: tokens, AST y tabla de símbolos |
| `./rp -i` | Modo interactivo (`:tabla`, `:memoria`, `:ast`, `:salir`) |
| `make prueba` | Ejecuta las pruebas automáticas |

## El lenguaje

Cada instrucción ocupa una línea. Los comentarios empiezan con `#`.

### Instrucciones

| Instrucción | Descripción | Equivalente en Python |
|---|---|---|
| `fijate que x es 19` | Declara una variable (sin `es`, vale `ni maiz`) | `x = 19` |
| `ponele 20 a x` | Asigna un nuevo valor | `x = 20` |
| `que onda x` | Muestra el tipo | `print(type(x))` |
| `contame x + 1` | Muestra un valor | `print(x + 1)` |
| `chamba f(m, n)` … `ya estuvo` | Declara una función | `def f(m, n):` |
| `devolveme valor` | Devuelve un valor desde una chamba | `return valor` |
| `f(1, 2)` | Llama una chamba | `f(1, 2)` |

### Tipos

El tipo se deduce del valor y puede cambiar al asignar otro valor (tipado dinámico).

| Tipo | Valores | Python |
|---|---|---|
| `cabal` | `42`, `-7` | `int` |
| `con vuelto` | `3.75`, `-0.5` | `float` |
| `chisme` | `"Ana"`, `'hola'` | `str` |
| `simonel` | `simon`, `nel` | `bool` |
| `ni maiz` | `ni maiz` | `None` |

### Operaciones

`+ - * /` con paréntesis y la precedencia habitual.

- Entre dos `cabal` el resultado es `cabal` y la división es entera (`7 / 2` = `3`).
- Con un `con vuelto` el resultado es `con vuelto` (`7 / 2.0` = `3.5`).
- `+` con un `chisme` concatena (`"Edad: " + 19` = `Edad: 19`).
- La resta entre nombres requiere espacios: `x - y`.

### Chambas (funciones)

```
chamba promedio(n1, n2)
    fijate que total es n1 + n2
    devolveme total / 2
ya estuvo

contame promedio(8, 10)        # 9
```

- Deben declararse antes de usarse y no pueden anidarse.
- Se llaman con tantos argumentos como parámetros tienen.
- Los parámetros y las variables internas son locales a la chamba.
- Pueden leer variables globales, pero no modificarlas.
- Sin `devolveme`, o con `devolveme` sin valor, devuelven `ni maiz`.

### Nombres

Empiezan con letra o `_` y pueden contener letras (incluidas `ñ` y tildes),
dígitos y `_`. Distinguen mayúsculas. No pueden ser palabras reservadas:
`fijate` `que` `es` `ponele` `a` `onda` `simon` `nel` `ni` `maiz` `contame`
`chamba` `devolveme` `ya` `estuvo`.

### Errores

El compilador reporta todos los errores con su fase, línea y una sugerencia.
El programa solo se ejecuta si no hay errores.

```
✘ Error léxico (línea 1, columna 12): '2do' no es un nombre válido: los nombres no pueden empezar con un dígito
✘ Error sintáctico (línea 6, columna 11): falta la palabra 'a': escriba 'ponele 20 a edad'
✘ Error semántico (línea 7): la chamba 'sumar' recibe 2 valor(es), pero se le pasaron 3
✘ Error de ejecución (línea 4): división entre cero
```

## Arquitectura

```
programa.rp → Léxico → tokens → Sintáctico → AST → Semántico → tabla de símbolos → Intérprete → salida
```

| Fase | Responsabilidad | Archivo |
|---|---|---|
| Léxico | Divide el texto en tokens. Las palabras y los símbolos están en las tablas `PALABRAS` y `SIMBOLOS` | `src/lexico.c` |
| Sintáctico | Valida la gramática y construye el AST (descenso recursivo) | `src/sintactico.c` |
| Semántico | Verifica declaraciones, ámbitos, tipos y llamadas; llena la tabla de símbolos | `src/semantico.c` |
| Intérprete | Ejecuta el AST con una pila de marcos para las chambas | `src/interprete.c` |

### Gramática (EBNF)

```
programa    → { linea } FIN
linea       → [ sentencia ] SALTO
sentencia   → declaracion | asignacion | consulta | contame | devolveme | chamba | llamada
declaracion → 'fijate' 'que' IDENT [ 'es' expresion ]
asignacion  → 'ponele' expresion 'a' IDENT
consulta    → 'que' 'onda' IDENT
contame     → 'contame' expresion
devolveme   → 'devolveme' [ expresion ]
chamba      → 'chamba' IDENT '(' [ IDENT { ',' IDENT } ] ')' SALTO { linea } 'ya' 'estuvo'
llamada     → IDENT '(' [ expresion { ',' expresion } ] ')'
expresion   → termino { ('+' | '-') termino }
termino     → factor { ('*' | '/') factor }
factor      → CABAL | CON_VUELTO | CHISME | 'simon' | 'nel' | 'ni' 'maiz'
            | IDENT | llamada | '(' expresion ')'
```

### Decisiones de diseño

- **Sin generadores (Flex/Bison):** el léxico y el parser están escritos a mano
  para que todo el código sea legible y no requiera herramientas adicionales.
- **Recuperación en modo pánico:** ante un error se descarta el resto de la
  línea y el análisis continúa, lo que permite reportar todos los errores.
- **Tabla de símbolos hash** (djb2), con un ámbito global y uno por chamba.
- **Tipado dinámico mediante unión etiquetada** (`ValorRP`): cada valor lleva su
  tipo. Los tipos que solo se conocen al ejecutar (parámetros, resultados de
  chambas) se verifican en el intérprete.

## Estructura del proyecto

```
src/
  main.c             Punto de entrada y modo interactivo
  compilador.c       Encadena las fases de análisis
  lexico.c, token.c  Fase léxica
  sintactico.c       Fase sintáctica
  ast.c              Nodos del AST
  semantico.c        Fase semántica
  tabla_simbolos.c   Tabla de símbolos
  interprete.c       Ejecución
  valor.c            Tipos y operaciones
  errores.c          Registro de errores
  reportes.c         Tablas y árbol en consola
  util.c             Memoria y UTF-8
pruebas/pruebas.c    Pruebas automáticas
ejemplos/            Programas de ejemplo
instalar/            Instaladores (macOS, Windows) y extensión de VS Code
```

## Extender el lenguaje

Para agregar una palabra reservada o un símbolo:

1. Declarar el token en el `enum` de `src/token.h` (las palabras reservadas, antes de `TK_CONTAME`).
2. Agregar su nombre en `NOMBRES` de `src/token.c`, en la misma posición.
3. Agregar una entrada en `PALABRAS` o `SIMBOLOS` de `src/lexico.c`.

Una instrucción nueva requiere además su nodo en `ast.h`, su regla en
`sintactico.c`, su verificación en `semantico.c`, su ejecución en
`interprete.c` y sus pruebas. La implementación de `contame` sirve como
referencia en cada archivo.

## Integración con el sistema

Opcional: muestra el logo en los archivos `.rp` y permite ejecutarlos con doble clic.

| Plataforma | Instalación |
|---|---|
| macOS | `./instalar/mac/instalar.sh` |
| Windows | `.\instalar\windows\instalar.bat` |
| VS Code | *Extensiones → ⋯ → Install from VSIX…* → `instalar/vscode/rp-lenguaje.vsix` (colores y ejecución con F5) |
