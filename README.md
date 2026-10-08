# Lenguaje RP

RP es un lenguaje de programación pequeño cuyas instrucciones usan expresiones
**chapinas** (el español coloquial de Guatemala). Por ejemplo, en vez de
`edad = 19` se escribe `fijate que edad es 19`.

Este repositorio contiene el **compilador de RP escrito en C**. Lee un programa
escrito en RP, revisa que esté bien escrito, muestra paso a paso lo que encontró
y lo ejecuta.

```
fijate que nombre es "Ana"     # crea una variable con un texto
fijate que edad es 19          # crea una variable con un número
ponele 20 a edad               # le cambia el valor
que onda edad                  # muestra su tipo: cabal (número entero)
```

> Proyecto académico del curso de compiladores. El objetivo es construir las
> bases de un compilador: analizador léxico, sintáctico y semántico, árbol de
> sintaxis abstracta (AST) y tabla de símbolos.

## Contenido

1. [Instalación en Mac](#1-instalación-en-mac)
2. [Instalación en Windows](#2-instalación-en-windows)
3. [Cómo ejecutar un programa](#3-cómo-ejecutar-un-programa)
4. [Aprenda RP paso a paso](#4-aprenda-rp-paso-a-paso)
5. [Referencia del lenguaje](#5-referencia-del-lenguaje)
6. [Cómo funciona el compilador](#6-cómo-funciona-el-compilador)
7. [Estructura del proyecto](#7-estructura-del-proyecto)
8. [Problemas comunes](#8-problemas-comunes)
9. [Glosario](#9-glosario)

---

## 1. Instalación en Mac

**Paso 1: abrir la Terminal.** Presione `Cmd + Espacio`, escriba `Terminal` y
presione Enter.

**Paso 2: instalar el compilador de C** (solo la primera vez). Escriba:

```bash
xcode-select --install
```

Aparecerá una ventana; presione **Instalar** y espere unos minutos. Si dice que
ya está instalado, puede seguir.

**Paso 3: entrar a la carpeta del proyecto.** Escriba `cd ` (con un espacio al
final), arrastre la carpeta `lenguaje-rp-c` desde el Finder a la Terminal y
presione Enter. Queda algo así:

```bash
cd /Users/su-usuario/Downloads/lenguaje-rp-c
```

**Paso 4: compilar.** Escriba:

```bash
make
```

Esto crea el programa `rp` dentro de la carpeta. Solo hay que repetirlo si se
modifica el código en `src/`.

**Paso 5: probar que funciona.**

```bash
./rp ejemplos/completo.rp
```

Al final de lo que se muestra deben aparecer estas dos líneas:

```
cabal
chisme
```

## 2. Instalación en Windows

**Paso 1: abrir PowerShell.** Presione la tecla Windows, escriba `PowerShell` y
presione Enter.

**Paso 2: instalar el compilador de C** (solo la primera vez). Escriba:

```powershell
winget install BrechtSanders.WinLibs.POSIX.UCRT
```

Esto instala `gcc` (de MinGW). Cuando termine, **cierre PowerShell y vuelva a
abrirlo** para que lo reconozca. Para comprobarlo, escriba `gcc --version`: debe
mostrar un número de versión.

> Si `winget` no existe en su computadora, descargue el instalador de
> [WinLibs](https://winlibs.com/) o de [MSYS2](https://www.msys2.org/) y agregue
> la carpeta `bin` de gcc a la variable de entorno `PATH`.

**Paso 3: entrar a la carpeta del proyecto.** Escriba `cd ` (con un espacio),
arrastre la carpeta `lenguaje-rp-c` desde el Explorador de archivos a PowerShell
y presione Enter:

```powershell
cd C:\Users\su-usuario\Downloads\lenguaje-rp-c
```

**Paso 4: compilar.** Escriba:

```powershell
.\compilar.bat
```

Debe mostrar `Listo: se creo rp.exe`.

**Paso 5: probar que funciona.**

```powershell
.\rp.exe ejemplos\completo.rp
```

Al final de lo que se muestra deben aparecer `cabal` y `chisme`.

---

## 3. Cómo ejecutar un programa

En Mac el programa se llama `./rp`; en Windows, `.\rp.exe`. Las rutas en Windows
usan `\` en lugar de `/`.

| Mac | Windows | Qué hace |
|---|---|---|
| `./rp archivo.rp` | `.\rp.exe archivo.rp` | Muestra todas las fases del compilador y ejecuta el programa |
| `./rp archivo.rp -s` | `.\rp.exe archivo.rp -s` | Solo ejecuta y muestra la salida del programa |
| `./rp -i` | `.\rp.exe -i` | Modo interactivo: escriba instrucciones una por una |
| `./rp -h` | `.\rp.exe -h` | Muestra la ayuda |
| `make prueba` | `.\compilar.bat prueba` | Corre las 47 pruebas automáticas |

### Crear su propio archivo `.rp`

Un programa de RP es un archivo de texto normal con extensión `.rp`.

- **Recomendado (Mac y Windows):** [Visual Studio Code](https://code.visualstudio.com/).
  Cree un archivo nuevo y guárdelo como `hola.rp` en la carpeta del proyecto.
- **Mac con TextEdit:** antes de escribir, elija *Formato → Convertir a texto
  normal*. Al guardar, escriba el nombre `hola.rp` y desmarque "Si no se indica
  ninguna extensión, usar .txt".
- **Windows con el Bloc de notas:** al guardar, en *Tipo* elija **Todos los
  archivos (\*.\*)** y en *Codificación* elija **UTF-8**. Si no, el archivo queda
  como `hola.rp.txt` y la `ñ` y las tildes no funcionarán.

### Modo interactivo

Sirve para probar el lenguaje sin crear archivos. Escriba una instrucción, presione
Enter y verá el resultado de inmediato:

```
rp> fijate que edad es 19
rp> que onda edad
cabal
rp> ponele "diecinueve" a edad
rp> que onda edad
chisme
rp> :salir
```

Comandos especiales del modo interactivo:

| Comando | Qué muestra |
|---|---|
| `:tabla` | La tabla de símbolos: todas las variables creadas, su tipo y dónde se usaron |
| `:memoria` | El valor actual de cada variable |
| `:ast` | El árbol de sintaxis de la última instrucción |
| `:salir` | Termina el modo interactivo |

### Qué muestra el compilador

Al ejecutar un archivo sin `-s`, se muestra en orden:

1. **Código fuente** con números de línea.
2. **Tabla de tokens** (fase léxica): cada pieza del código con su tipo, línea y columna.
3. **AST** (fase sintáctica): el programa dibujado como árbol y como tabla.
4. **Tabla de símbolos** (fase semántica): cada variable con su tipo e historial.
5. **Ejecución**: lo que imprime el programa y el valor final de cada variable.

Si hay errores, se muestran todos al final y el programa no se ejecuta.

---

## 4. Aprenda RP paso a paso

Puede escribir estos ejemplos en un archivo `.rp` o en el modo interactivo.

### 4.1 Crear una variable

Las variables se crean con `fijate que`, como cuando uno dice "fijate que…" para
contar algo nuevo. La palabra `es` le da su primer valor:

```
fijate que nombre es "Ana"
fijate que edad es 19
```

También se puede crear vacía. En ese caso vale `ni maiz` (nada):

```
fijate que apodo
```

### 4.2 Cambiar el valor

Para cambiar el valor de una variable que ya existe se usa `ponele <valor> a <nombre>`,
como diría un chapín: "ponele 20 a la edad".

```
ponele 20 a edad
ponele "La Seño" a apodo
```

El valor también puede ser otra variable; se copia su valor:

```
fijate que copia
ponele edad a copia
```

### 4.3 Preguntar el tipo

`que onda <nombre>` muestra qué tipo de dato guarda una variable:

```
fijate que precio es 12.50
que onda precio
```

Salida:

```
con vuelto
```

### 4.4 Una variable puede cambiar de tipo

RP adivina el tipo por la forma del valor, y si se le pone un valor de otro tipo,
la variable cambia de tipo (igual que en Python):

```
fijate que dato es 7
que onda dato          # cabal
ponele 7.5 a dato
que onda dato          # con vuelto
ponele "siete" a dato
que onda dato          # chisme
```

### 4.5 Comentarios

Todo lo que va después de `#` se ignora. Sirve para dejar notas:

```
# Este programa guarda la nota del curso
fijate que nota es 85    # sobre 100
```

### 4.6 Un programa completo

```
# Declaraciones
fijate que nombre es "Ana"
fijate que edad es 19
fijate que nota es 8.5
fijate que aprobado es simon
fijate que apodo              # queda en ni maiz

# Asignaciones
ponele 20 a edad
ponele "La Seño" a apodo
ponele nel a aprobado

# Identificación de tipos
que onda edad                 # muestra: cabal
que onda apodo                # muestra: chisme
```

Este programa está en `ejemplos/completo.rp`. Hay más ejemplos en la carpeta
`ejemplos/`:

| Archivo | Qué muestra |
|---|---|
| `completo.rp` | El programa de arriba |
| `tipos.rp` | Los cinco tipos de dato y los cambios de tipo |
| `errores_lexicos.rp` | Errores de escritura: nombres o números mal formados, símbolos no permitidos |
| `errores_sintacticos.rp` | Instrucciones mal armadas: palabras faltantes o fuera de orden |
| `errores_semanticos.rp` | Instrucciones bien escritas pero sin sentido: usar una variable que no existe |

---

## 5. Referencia del lenguaje

### Instrucciones

Cada instrucción va en su propia línea.

| Instrucción | Para qué sirve | Ejemplo | Equivalente en Python |
|---|---|---|---|
| `fijate que <nombre>` | Crear una variable vacía (vale `ni maiz`) | `fijate que apodo` | `apodo = None` |
| `fijate que <nombre> es <valor>` | Crear una variable con un valor | `fijate que edad es 19` | `edad = 19` |
| `ponele <valor> a <nombre>` | Cambiar el valor de una variable que ya existe | `ponele 20 a edad` | `edad = 20` |
| `que onda <nombre>` | Mostrar el tipo actual de una variable | `que onda edad` | `print(type(edad))` |

### Tipos de dato

El programador nunca escribe el tipo: RP lo deduce por la forma del valor.

| Tipo en RP | Qué guarda | Ejemplos | En Python | En C | Por qué se llama así |
|---|---|---|---|---|---|
| `cabal` | Números enteros | `42`, `-7` | `int` | `int` | "Cabal" significa exacto |
| `con vuelto` | Números con decimales | `3.75`, `-0.5` | `float` | `double` | Como un pago que deja vuelto |
| `chisme` | Texto entre comillas | `"Ana"`, `'hola'` | `str` | `char*` | El chisme es pura plática |
| `simonel` | Verdadero o falso | `simon`, `nel` | `bool` | `bool` | De "simón" (sí) y "nel" (no) |
| `ni maiz` | Ausencia de valor | `ni maiz` | `None` | `NULL` | "Ni maíz" es no haber nada |

Para reconocer el tipo, RP revisa en este orden: si tiene comillas → `chisme`;
si es `simon`, `nel` o `ni maiz`; si tiene punto → `con vuelto`; si son solo
dígitos → `cabal`. Detalles:

- Un decimal necesita dígitos a ambos lados del punto: `0.5` sí; `.5` y `5.` no.
- `"simon"` entre comillas es un `chisme`, no un `simonel`.

### Reglas para los nombres de variables

- Empiezan con una letra o con `_`; después pueden llevar letras, dígitos y `_`.
- Se permiten la `ñ` y las tildes: `año`, `canción`.
- No pueden tener espacios ni símbolos como `-`, `@` o `$`.
- No pueden ser una **palabra reservada**: `fijate`, `que`, `es`, `ponele`, `a`,
  `simon`, `nel`, `ni`, `maiz`, `onda`.
- Una variable se crea una sola vez con `fijate que`; después se cambia con `ponele`.
- Las mayúsculas importan: `Edad` y `edad` son variables distintas, y `Fijate`
  no es una palabra reservada.

| Código | ¿Válido? | Motivo |
|---|---|---|
| `fijate que año es 2026` | Sí | La `ñ` está permitida |
| `fijate que _total` | Sí | Puede empezar con `_` |
| `fijate que 2do es 5` | No | El nombre empieza con un número |
| `fijate que mi pisto es 100` | No | El nombre tiene un espacio |
| `fijate que onda es 3` | No | `onda` es palabra reservada |
| `edad es 20` | No | Falta `fijate que` para crear la variable |
| `fijate edad es 20` | No | La frase completa es `fijate que` |
| `ponele a edad 20` | No | El valor va antes de la `a` |
| `ponele 20 edad` | No | Falta la palabra `a` |
| `ponele 500 a sueldo` | No | `sueldo` no fue creada antes |
| `edad = 20` | No | RP no usa `=` |

### Mensajes de error

Cuando un programa tiene errores, el compilador los muestra **todos a la vez**,
con la fase, la línea, la columna y una sugerencia para corregirlos. El programa
solo se ejecuta si no tiene ningún error.

```
✘ Error léxico (línea 1, columna 12): '2do' no es un nombre válido: los nombres no pueden empezar con un dígito
✘ Error sintáctico (línea 6, columna 11): falta la palabra 'a': escriba 'ponele 20 a edad'
✘ Error semántico (línea 3): no se puede asignar a 'sueldo' porque no ha sido declarada; declárela primero con 'fijate que sueldo'
```

---

## 6. Cómo funciona el compilador

Un compilador trabaja por **fases**: cada una recibe el resultado de la anterior.
Si una fase encuentra errores, las siguientes no se ejecutan.

```
programa.rp ─► LÉXICO ─► tokens ─► SINTÁCTICO ─► AST ─► SEMÁNTICO ─► tabla de símbolos ─► INTÉRPRETE ─► salida
```

| Fase | Qué hace, en palabras simples | Ejemplo de error que detecta | Archivo |
|---|---|---|---|
| **1. Análisis léxico** | Lee el texto letra por letra y lo separa en piezas llamadas *tokens* (palabras, números, textos) | `2do` (un nombre no empieza con número) | `src/lexico.c` |
| **2. Análisis sintáctico** | Revisa que los tokens formen instrucciones válidas según la gramática y construye el *AST* | `ponele 20 edad` (falta la `a`) | `src/sintactico.c` |
| **3. Análisis semántico** | Revisa que las instrucciones tengan sentido: que las variables existan y qué tipo tiene cada una. Llena la *tabla de símbolos* | `ponele 500 a sueldo` (`sueldo` no existe) | `src/semantico.c` |
| **4. Ejecución** | Recorre el AST y ejecuta cada instrucción | — | `src/interprete.c` |

### Gramática

La gramática define qué combinaciones de tokens son instrucciones válidas. Está
escrita en notación EBNF: `|` significa "o", `[ ]` significa "opcional" y
`{ }` significa "cero o más veces".

```
programa    → { linea } FIN
linea       → [ sentencia ] SALTO
sentencia   → declaracion | asignacion | consulta
declaracion → 'fijate' 'que' IDENT [ 'es' valor ]
asignacion  → 'ponele' valor 'a' IDENT
consulta    → 'que' 'onda' IDENT
valor       → CABAL | CON_VUELTO | CHISME | 'simon' | 'nel' | 'ni' 'maiz' | IDENT
```

Cada token se reconoce con una expresión regular:

```
IDENT       [letra _][letra dígito _]*      nombre de variable (que no sea palabra reservada)
CABAL       -?[0-9]+                        número entero
CON_VUELTO  -?[0-9]+\.[0-9]+                número decimal
CHISME      "[^"\n]*"  |  '[^'\n]*'         texto entre comillas
```

### Decisiones de diseño

- **Parser de descenso recursivo:** cada regla de la gramática es una función
  de C (`declaracion()`, `asignacion()`, `valor()`...). Está escrito a mano, sin
  herramientas generadoras como Lex/Yacc o ANTLR.
- **Recuperación de errores en "modo pánico":** al encontrar un error, el
  compilador lo anota, se salta el resto de esa línea y sigue con la siguiente.
  Por eso puede mostrar todos los errores de una sola vez.
- **Tabla de símbolos con tabla hash:** cada nombre se convierte en un número
  (función hash *djb2*) que indica dónde guardarlo, para encontrar las variables
  rápido. Guarda nombre, tipo actual, ámbito, línea de declaración, líneas donde
  se asigna y se usa, e historial de tipos.
- **Tipado dinámico en C:** C exige que cada variable tenga un tipo fijo, pero en
  RP una variable puede pasar de `cabal` a `chisme`. Por eso cada valor de RP se
  guarda junto con una etiqueta que indica su tipo (una *unión etiquetada*,
  `ValorRP` en `src/valor.h`). `que onda` solo lee esa etiqueta.
- **Soporte de UTF-8:** el analizador léxico reconoce la `ñ` y las vocales con
  tilde en los nombres. En Windows, el programa activa UTF-8 en la consola para
  que se vean bien las tildes y los bordes de las tablas.

---

## 7. Estructura del proyecto

```
lenguaje-rp-c/
├── Makefile              Compilar en Mac (make) y correr pruebas (make prueba)
├── compilar.bat          Compilar en Windows (compilar.bat) y correr pruebas (compilar.bat prueba)
├── README.md             Este archivo
├── ejemplos/             Programas de ejemplo escritos en RP (.rp)
├── pruebas/
│   └── pruebas.c         Pruebas automáticas de todas las reglas
└── src/                  Código fuente del compilador
    ├── main.c            Programa principal: lee el archivo, muestra resultados, modo interactivo
    ├── compilador.c      Une las fases: léxico → sintáctico → semántico
    ├── lexico.c          Fase 1: análisis léxico (texto → tokens)
    ├── sintactico.c      Fase 2: análisis sintáctico (tokens → AST)
    ├── ast.c             Estructura de los nodos del AST
    ├── semantico.c       Fase 3: análisis semántico (revisa el AST)
    ├── tabla_simbolos.c  Tabla de símbolos (tabla hash)
    ├── interprete.c      Fase 4: ejecuta el programa
    ├── reportes.c        Dibuja las tablas y el árbol en la terminal
    ├── token.c           Tipos de token y palabras reservadas
    ├── valor.c           Tipos de dato de RP y sus valores
    ├── errores.c         Lista de errores y sus mensajes
    └── util.c            Funciones de apoyo (memoria, texto UTF-8)
```

Cada archivo `.c` tiene un `.h` con el mismo nombre que declara sus funciones.

---

## 8. Problemas comunes

| Problema | Solución |
|---|---|
| Mac: `make: command not found` o `xcrun: error` | Falta el compilador. Ejecute `xcode-select --install` |
| Mac: `zsh: command not found: rp` | Escriba `./rp`, con `./` al inicio |
| Mac: `zsh: permission denied: ./rp` | Vuelva a compilar con `make` |
| Windows: `No se encontro gcc` o `gcc no se reconoce` | Instale gcc (sección 2) y **cierre y vuelva a abrir** PowerShell |
| Windows: `rp.exe no se reconoce` | Escriba `.\rp.exe`, con `.\` al inicio, y compile antes con `.\compilar.bat` |
| `No se pudo abrir 'archivo.rp'` | Revise el nombre y que esté en la carpeta actual. En Windows, que no se haya guardado como `archivo.rp.txt` |
| La `ñ` o las tildes dan error `símbolo no permitido` | Guarde el archivo con codificación **UTF-8** |
| Los bordes de las tablas se ven como símbolos raros (Windows) | Use PowerShell o Windows Terminal en lugar de la consola antigua (cmd) |

---

## 9. Glosario

| Término | Significado |
|---|---|
| **Compilador** | Programa que lee código escrito en un lenguaje, revisa que esté correcto y lo traduce o ejecuta |
| **Token** | La pieza mínima con significado: una palabra reservada, un nombre, un número o un texto |
| **Lexema** | El texto exacto de un token tal como aparece en el código (por ejemplo `19`) |
| **Gramática** | Las reglas que dicen en qué orden pueden ir los tokens para formar instrucciones |
| **AST** (árbol de sintaxis abstracta) | Representación del programa en forma de árbol: la raíz es el programa, las ramas son las instrucciones y las hojas son los valores |
| **AST decorado** | El AST después de que el análisis semántico le anota el tipo de cada nodo |
| **Tabla de símbolos** | Registro de todas las variables del programa con su tipo y dónde se usan |
| **Ámbito** | La parte del programa donde existe una variable. En RP todas son globales |
| **Tipado dinámico** | El tipo de una variable lo determina el valor que guarda y puede cambiar |
| **Intérprete** | Parte que ejecuta el programa directamente, instrucción por instrucción |
