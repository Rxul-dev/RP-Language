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
| `ponele 95 a notas[1]` | Cambia una posición de un chorizo | `notas[1] = 95` |
| `metele 70 a notas` | Agrega un valor al final de un chorizo | `notas.append(70)` |
| `que onda x` | Muestra el tipo | `print(type(x))` |
| `contame x + 1` | Muestra un valor | `print(x + 1)` |
| `si x >= 18` … `ya estuvo` | Ejecuta el bloque si la condición es `simon` | `if x >= 18:` |
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
| `chorizo` | `[90, "Ana", simon]`, `[]` | `list` |

### Operaciones

`+ - * /` con paréntesis y la precedencia habitual.

- Entre dos `cabal` el resultado es `cabal` y la división es entera (`7 / 2` = `3`).
- Con un `con vuelto` el resultado es `con vuelto` (`7 / 2.0` = `3.5`).
- `+` con un `chisme` concatena (`"Edad: " + 19` = `Edad: 19`).
- La resta entre nombres requiere espacios: `x - y`.
- Comparaciones: `==  !=  <  >  <=  >=`, que dan `simon` o `nel`. `<`, `>`, `<=` y
  `>=` solo comparan números; `==` y `!=` aceptan cualquier valor (`2 == 2.0` es `simon`).

### Condicionales

```
si edad >= 18
    contame "Es mayor de edad"
ya estuvo
```

- La condición debe dar `simon` o `nel`.
- Un `si` puede ir dentro de otro `si` o de una chamba; un `devolveme` adentro termina la chamba.
- Las variables declaradas dentro del bloque se ven afuera, pero solo existen si
  la condición se cumplió (como en Python).

### Chorizos (arreglos)

```
fijate que notas es [90, 85, 70]
contame notas[0]               # 90
ponele 95 a notas[1]           # [90, 95, 70]
metele 60 a notas              # [90, 95, 70, 60]
contame cuantos notas          # 4
```

- Las posiciones empiezan en `0`; una posición que no existe es un error de ejecución.
- Guardan valores de cualquier tipo, incluso otros chorizos: `tabla[1][0]`.
- `cuantos` da la cantidad de valores de un chorizo o de letras de un chisme.
- `==` y `!=` comparan los valores en orden; `+` con un chisme lo une como texto.
- Se copian completos al asignarlos: después de `fijate que b es a`, cambiar `b` no cambia `a`.
- Una chamba puede recibirlos y recorrerlos llamándose a sí misma (ver `ejemplos/chorizos.rp`).

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
`si` `chamba` `devolveme` `ya` `estuvo` `metele` `cuantos`.

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
| Léxico | Divide el texto en tokens. Las palabras y los símbolos están en las tablas `PALABRAS` y `SIMBOLOS` | `src/fases/lexico.c` |
| Sintáctico | Valida la gramática y construye el AST (descenso recursivo) | `src/fases/sintactico.c` |
| Semántico | Verifica declaraciones, ámbitos, tipos y llamadas; llena la tabla de símbolos | `src/fases/semantico.c` |
| Intérprete | Ejecuta el AST con una pila de marcos para las chambas | `src/fases/interprete.c` |

Cada archivo de `src/fases/` tiene solo lo que comparten todas las instrucciones y
elige qué instrucción toca. Cada instrucción vive en su propio archivo de
`src/instrucciones/` con sus tres partes, una por fase:

```c
Nodo *parsear_contame(Parser *p);                    // sintáctico: arma su nodo del AST
void verificar_contame(Semantico *sem, Nodo *nodo);  // semántico: revisa tipos y nombres
void ejecutar_contame(Interprete *in, const Nodo *nodo);  // intérprete: la ejecuta
```

### Gramática (EBNF)

```
programa    → { linea } FIN
linea       → [ sentencia ] SALTO
sentencia   → declaracion | asignacion | consulta | contame | devolveme | si | chamba
            | metele | llamada
declaracion → 'fijate' 'que' IDENT [ 'es' expresion ]
asignacion  → 'ponele' expresion 'a' IDENT [ '[' expresion ']' ]
consulta    → 'que' 'onda' IDENT
contame     → 'contame' expresion
devolveme   → 'devolveme' [ expresion ]
si          → 'si' expresion SALTO bloque
chamba      → 'chamba' IDENT '(' [ IDENT { ',' IDENT } ] ')' SALTO bloque
metele      → 'metele' expresion 'a' IDENT
bloque      → { linea } 'ya' 'estuvo'
llamada     → IDENT '(' [ expresion { ',' expresion } ] ')'
expresion   → suma [ ('==' | '!=' | '<' | '>' | '<=' | '>=') suma ]
suma        → termino { ('+' | '-') termino }
termino     → factor { ('*' | '/') factor }
factor      → primario { '[' expresion ']' }
primario    → CABAL | CON_VUELTO | CHISME | 'simon' | 'nel' | 'ni' 'maiz'
            | IDENT | llamada | chorizo | 'cuantos' factor | '(' expresion ')'
chorizo     → '[' [ expresion { ',' expresion } ] ']'
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
  main.c                Punto de entrada: lee los argumentos y ejecuta un archivo
  interactivo.c         Modo interactivo (rp -i)
  compilador.c          Encadena las fases de análisis
  reportes.c            Tablas y árbol en consola
  base/
    util.c              Memoria, listas que crecen y UTF-8
    valor.c             Tipos y operaciones (incluye los chorizos)
    errores.c           Registro de errores
  fases/
    token.c, lexico.c   Fase léxica
    ast.c               Nodos del AST
    sintactico.c        Fase sintáctica: lo común a todas las reglas
    tabla_simbolos.c    Tabla de símbolos
    semantico.c         Fase semántica: lo común a todas las verificaciones
    interprete.c        Ejecución: la memoria y qué instrucción ejecutar
  instrucciones/
    instrucciones.h     Las funciones de cada instrucción
    expresiones.c       Valores y operaciones: 2 + x * 3, edad >= 18
    declaracion.c       fijate que
    asignacion.c        ponele
    consulta.c          que onda
    contame.c           contame
    si.c                si ... ya estuvo
    chambas.c           chamba, devolveme y llamadas
    chorizos.c          [1, 2], lista[0], metele y cuantos
pruebas/pruebas.c       Pruebas automáticas
ejemplos/            Programas de ejemplo
instalar/            Instaladores (macOS, Windows) y extensión de VS Code
```

## Extender el lenguaje

Para agregar una palabra reservada o un símbolo:

1. Declarar el token en el `enum` de `src/fases/token.h` (las palabras reservadas, antes de `TK_IDENT`).
2. Agregar su nombre en `NOMBRES` de `src/fases/token.c`.
3. Agregar una entrada en `PALABRAS` o `SIMBOLOS` de `src/fases/lexico.c`.

Una instrucción nueva va en su propio archivo de `src/instrucciones/` con sus
tres funciones (`parsear_`, `verificar_`, `ejecutar_`), declaradas en
`instrucciones.h`. Además necesita su nodo en `fases/ast.h`, una línea en cada
`switch` que elige la instrucción (`sentencia()` en `sintactico.c`,
`semantico_visitar()` en `semantico.c` e `interprete_ejecutar_sentencia()` en
`interprete.c`) y sus pruebas. `contame.c` es el ejemplo más sencillo.

Antes de escribir código nuevo, revise los ayudantes que ya existen:

| Para… | Use |
|---|---|
| `<palabra> <valor>` (como `contame`) | `instruccion_con_valor()` |
| `<palabra> <valor> a <nombre>` (como `ponele`, `metele`) | `instruccion_valor_a_nombre()` |
| Una lista separada por comas (argumentos, parámetros, chorizos) | `leer_lista()` |
| Un bloque que termina en `ya estuvo` | `bloque()` |
| Un error sintáctico, semántico o de ejecución | `error_en()`, `error_semantico()`, `error_ejecucion()` |
| Una variable que se lee o que se cambia | `usar()`, `variable_a_cambiar()` |
| Una lista que crece | `crecer()` |

## Integración con el sistema

Opcional: muestra el logo en los archivos `.rp` y permite ejecutarlos con doble clic.

| Plataforma | Instalación |
|---|---|
| macOS | `./instalar/mac/instalar.sh` |
| Windows | `.\instalar\windows\instalar.bat` |
| VS Code | *Extensiones → ⋯ → Install from VSIX…* → `instalar/vscode/rp-lenguaje.vsix` (colores y ejecución con F5) |
