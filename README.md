# Analizador Sintáctico para Pascal

Implementación en **C** de un analizador sintáctico (parser) para un subconjunto del lenguaje **Pascal**. Realiza análisis léxico y sintáctico, valida la estructura del programa y reporta errores de sintaxis con su ubicación.

A **Pascal** syntactic analyzer (parser) written in **C**. It performs lexical and syntactic analysis, validates the program structure and reports syntax errors with their location.

> Complementa a [**Analizador_Semantico**](https://github.com/JoseBurgoss/Analizador_Semantico), que añade el análisis semántico (tipos, ámbitos, funciones).

---

## Características / Features
- Análisis léxico (tokens: identificadores, palabras reservadas, operadores, literales)
- Análisis sintáctico descendente del programa Pascal
- Validación de declaraciones de variables, funciones y procedimientos
- Validación de estructuras de control (`if`, `while`, `for`, `begin/end`)
- Validación de expresiones y operadores
- Reporte de errores indicando línea/posición

## Estructura del proyecto
```
Analizador_Sintactico/
├── Sintactico2.c        # Código fuente del analizador en C
├── Sintactico2.exe      # Ejecutable compilado (Windows)
├── codigo_pascal.txt    # Código Pascal de ejemplo para analizar
└── damechance.txt       # Casos de prueba adicionales
```

## Cómo compilar y ejecutar
Con **GCC**:
```bash
gcc Sintactico2.c -o Sintactico2
./Sintactico2
```
O ejecuta directamente el binario incluido en Windows:
```powershell
.\Sintactico2.exe
```
El programa lee un archivo de código Pascal (por ejemplo `codigo_pascal.txt`) y muestra el resultado del análisis y los errores detectados.

## Ejemplo de entrada
```pascal
program Ejemplo;
var
    x: integer;
    y: string;

procedure Saludar;
begin
    writeln('Hola');
end;

function Sumar(a, b: integer): integer;
begin
    Sumar := a + b;
end;
```

## Tecnologías
- **Lenguaje:** C
- **Paradigma:** análisis léxico + parser descendente

---

## 📫 Contacto / Contact
- **Autor:** José Burgos
- ✉️ joseburgos153@gmail.com
- 🔗 [LinkedIn](https://www.linkedin.com/in/jose-burgos-/) · [GitHub](https://github.com/JoseBurgoss)