#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>



const char* palabras_clave[] = {"begin", "end", "then", "else", "while", "do", "for", "to", "downto", 
                                "repeat", "until", "case", "of", "const", "type", "record", "array", "var"};

const char* tipos_validos[] = {"integer", "string", "real", "boolean", "char"};

const char* operadoresAritmeticos[] = {"*", "/","mod","+","-"};

const char* operadoresDeComparacion[] = {"<>", "<=", ">=", "<", ">", "="};


typedef struct Nodo {
    char tipo[20];      
    char valor[50];      
    struct Nodo** hijos;  
    int num_hijos;       
} Nodo;

Nodo* crear_nodo(const char* tipo, const char* valor);
void agregar_hijo(Nodo* padre, Nodo* hijo);
bool end_with_semicolon(const char* str);
void removeSpaces(char *str);
void trim(char *str);
bool contiene_elemento(const char *cadena, const char *array[], int tam_array);
void trim_semicolon(char *str);
int contar_elementos(char **array);
void toLowerCase(char *str);
char **split_function(const char *str, int *count);
char *extraer_parentesis(const char *str);
char **split(const char *str, const char *delim, int *count);
void mostrar_error(const char* mensaje, int linea, const char* detalle);
void mostrar_advertencia(const char* mensaje, int linea);
int es_tipo_valido(const char* tipo);
void analizar_palabra_clave(Nodo* arbol, const char* linea, int *num_linea, bool fromF_Or_P, FILE* archivo);
void analizar_cabecera_funcion(Nodo* arbol, char* linea, int num_linea, char* nombre_funcion);
void analizar_funcion(Nodo* arbol, char* linea, int* num_linea, FILE* archivo, char* nombre_funcion);
void analizar_expresion(Nodo* arbol, char* expr, int num_linea);
void analizar_asignacion(Nodo* arbol, const char* linea, int num_linea);
void imprimir_arbol(Nodo* nodo, int nivel);
void analizar_writeln(Nodo* arbol, const char* linea, int num_linea);
void analizar_if(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo);
void analizar_while(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo);
void analizar_for(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo);



Nodo* crear_nodo(const char* tipo, const char* valor) {
    // ////printf("CREAR NODO\n");
    // ////printf("TIPO: %s\n", tipo);
    // ////printf("VALOR: %s\n", valor);
    Nodo* nodo = (Nodo*)malloc(sizeof(Nodo));
    strcpy(nodo->tipo, tipo);
    strcpy(nodo->valor, valor);
    nodo->hijos = NULL;
    nodo->num_hijos = 0;
    return nodo;
}

void agregar_hijo(Nodo* padre, Nodo* hijo) {
    padre->num_hijos++;
    padre->hijos = (Nodo**)realloc(padre->hijos, padre->num_hijos * sizeof(Nodo*));
    padre->hijos[padre->num_hijos - 1] = hijo;
}

void extraer_condicion_while(const char* linea, char* condicion) {
    const char* inicio_while = strstr(linea, "while");  // Buscar "while"
    if (inicio_while != NULL) {
        // Avanzar al siguiente carácter después de "while"
        inicio_while += 5;

        // Saltar posibles espacios
        while (*inicio_while && isspace(*inicio_while)) {
            inicio_while++;
        }

        // Encontrar la palabra "do" que marca el final de la condición
        const char* fin_do = strstr(inicio_while, "do");
        if (fin_do != NULL) {
            // Copiar la parte entre "while" y "do"
            size_t longitud = fin_do - inicio_while;
            strncpy(condicion, inicio_while, longitud);
            condicion[longitud] = '\0';  // Terminar la cadena correctamente
        } else {
            //printf("Error: 'do' no encontrado en la línea.\n");
            condicion[0] = '\0';  // Devolver cadena vacía en caso de error
        }
    } else {
        //printf("Error: 'while' no encontrado en la línea.\n");
        condicion[0] = '\0';  // Devolver cadena vacía en caso de error
    }
}

void obtenerNombreFuncion(const char *cabecera, char *nombre, size_t tam) {
    // Avanzar hasta saltar espacios en blanco iniciales
    const char *ptr = cabecera;
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }
    
    // Verificar que la cabecera comience con "function"
    const char *palabra = "function";
    size_t len = strlen(palabra);
    
    if (strncasecmp(ptr, palabra, len) != 0) {
        // No es una cabecera válida de función
        nombre[0] = '\0';
        return;
    }
    
    // Mover el puntero después de la palabra "function"
    ptr += len;
    
    // Saltar espacios en blanco posteriores
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }
    
    // El nombre comienza aquí y termina en el primer espacio o al encontrar '('
    size_t i = 0;
    while (*ptr && !isspace(*ptr) && *ptr != '(' && i < tam - 1) {
        nombre[i++] = *ptr;
        ptr++;
    }
    nombre[i] = '\0';
    ////printf("Devolvere el nombre de la funcion: %s\n", nombre);
}

void obtenerNombreProcedure(const char *cabecera, char *nombre, size_t tam) {
    // Avanzar hasta saltar espacios en blanco iniciales
    const char *ptr = cabecera;
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }
    
    // Verificar que la cabecera comience con "procedure"
    const char *palabra = "procedure";
    size_t len = strlen(palabra);
    
    if (strncasecmp(ptr, palabra, len) != 0) {
        // No es una cabecera válida de procedure
        nombre[0] = '\0';
        return;
    }
    
    // Mover el puntero después de la palabra "procedure"
    ptr += len;
    
    // Saltar espacios en blanco posteriores
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }
    
    // El nombre comienza aquí y termina en el primer espacio o al encontrar '(' o ';'
    size_t i = 0;
    while (*ptr && !isspace(*ptr) && *ptr != '(' && *ptr != ';' && i < tam - 1) {
        nombre[i++] = *ptr;
        ptr++;
    }
    nombre[i] = '\0';
}

void contenidoWriteln(char* linea, char* contenido) {
    const char *ptr = linea;
    while (*ptr && *ptr != '(') {
        ptr++;
    }
    ptr++;
    size_t i = 0;
    while (*ptr && *ptr != ')') {
        contenido[i++] = *ptr;
        ptr++;
    }
    contenido[i] = '\0';
}

bool starts_with(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) {
        return false;
    }
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) {
        return false;
    }
    return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

void extraer_condicion_if(const char* linea, char* condicion) {
    const char* inicio_if = strstr(linea, "if");  // Buscar "if"
    if (inicio_if != NULL) {
        // Avanzar al siguiente carácter después de "if"
        inicio_if += 2;

        // Saltar posibles espacios
        while (*inicio_if && isspace(*inicio_if)) {
            inicio_if++;
        }

        // Encontrar la palabra "then" que marca el final de la condición
        const char* fin_then = strstr(inicio_if, "then");
        if (fin_then != NULL) {
            // Copiar la parte entre "if" y "then"
            size_t longitud = fin_then - inicio_if;
            strncpy(condicion, inicio_if, longitud);
            condicion[longitud] = '\0';  // Terminar la cadena correctamente
        } else {
            //printf("Error: 'then' no encontrado en la línea.\n");
            condicion[0] = '\0';  // Devolver cadena vacía en caso de error
        }
    } else {
        //printf("Error: 'if' no encontrado en la línea.\n");
        condicion[0] = '\0';  // Devolver cadena vacía en caso de error
    }
}

void extraer_condicion_for(const char* linea, char* inicializacion, char* operador_control, char* final) {
    const char* inicio_for = strstr(linea, "for");  // Buscar "for"
    if (inicio_for != NULL) {
        // Avanzar al siguiente carácter después de "for"
        inicio_for += 3;

        // Saltar posibles espacios
        while (*inicio_for && isspace(*inicio_for)) {
            inicio_for++;
        }

        // Buscar el operador de asignación ":="
        const char* asignacion = strstr(inicio_for, ":=");
        if (asignacion != NULL) {
            asignacion += 2; // Avanzar después de ":="
            
            // Saltar espacios después de ":="
            while (*asignacion && isspace(*asignacion)) {
                asignacion++;
            }

            // Buscar "to" o "downto" que marcan el final de la inicialización
            const char* fin_inicializacion = strstr(asignacion, " to");
            if (fin_inicializacion == NULL) {
                fin_inicializacion = strstr(asignacion, " downto");
            }

            if (fin_inicializacion != NULL) {
                size_t longitud = fin_inicializacion - inicio_for;
                strncpy(inicializacion, inicio_for, longitud);
                inicializacion[longitud] = '\0';  // Terminar la cadena correctamente
                inicio_for = fin_inicializacion; // Mover el puntero al inicio de "to" o "downto"
            } else {
                //printf("Error: 'to' o 'downto' no encontrado después de la inicialización.\n");
                inicializacion[0] = '\0';
                return;
            }
        } else {
            //printf("Error: ':=' no encontrado en la inicialización.\n");
            inicializacion[0] = '\0';
            return;
        }

        // Saltar posibles espacios antes del operador de control
        while (*inicio_for && isspace(*inicio_for)) {
            inicio_for++;
        }

        // Buscar el operador de control ("to" o "downto")
        if (strncmp(inicio_for, "to", 2) == 0) {
            strcpy(operador_control, "to");
            inicio_for += 2;  // Avanzar después de "to"
        } else if (strncmp(inicio_for, "downto", 6) == 0) {
            strcpy(operador_control, "downto");
            inicio_for += 6;  // Avanzar después de "downto"
        } else {
            //printf("Error: No se encontró el operador 'to' o 'downto'.\n");
            operador_control[0] = '\0';
            return;
        }

        // Saltar posibles espacios después del operador de control
        while (*inicio_for && isspace(*inicio_for)) {
            inicio_for++;
        }

        // El valor final es lo que sigue después del operador de control hasta que encontramos "do"
        const char* fin_final = strstr(inicio_for, " do");
        if (fin_final != NULL) {
            size_t longitud = fin_final - inicio_for;
            strncpy(final, inicio_for, longitud);
            final[longitud] = '\0';  // Terminar la cadena correctamente
        } else {
            //printf("Error: 'do' no encontrado al final del ciclo.\n");
            final[0] = '\0';
        }
    } else {
        //printf("Error: 'for' no encontrado en la línea.\n");
        inicializacion[0] = '\0';
        operador_control[0] = '\0';
        final[0] = '\0';
    }
}

bool end_with_semicolon(const char* str) {
    trim((char*)str);
    //printf("STR: %s\n", str);
    size_t len = strlen(str);
    if (str[len - 1] != ';') {
        return false;
    }
    return true;
}

void removeSpaces(char *str) {
    char *dst = str; // Puntero para escribir los caracteres no espacios
    while (*str) {
        if (!isspace((unsigned char)*str)) { // Verifica si el caracter NO es un espacio en blanco
            *dst++ = *str; // Copia el caracter
        }
        str++;
    }
    *dst = '\0'; // Termina la cadena resultante
}

void trim(char *str) {
    // ////printf("TRIM\n");
    // ////printf("str: %s\n", str);
    char *start = str;
    char *end;

    // Mover el puntero de inicio hasta el primer carácter no blanco
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // Si la cadena es completamente espacios en blanco, dejarla vacía
    if (*start == '\0') {
        *str = '\0';
        return;
    }

    // Encontrar el final de la cadena
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    // Ajustar la cadena al inicio y añadir terminador nulo
    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

bool contiene_elemento(const char *cadena, const char *array[], int tam_array) {
    for (int i = 0; i < tam_array; i++) {
        if (strstr(cadena, array[i]) != NULL) { // strstr devuelve un puntero si encuentra la subcadena
            //printf("La cadena %s contiene %s\n", cadena, array[i]);
            return true;  // Encontró al menos una coincidencia
        }
    }
    return false;  // No encontró ninguna coincidencia
}

void trim_semicolon(char *str) {
    int len = strlen(str);

    // Recorrer la cadena desde el final y eliminar los ';'
    while (len > 0 && str[len - 1] == ';') {
        str[len - 1] = '\0';
        len--;
    }
}

int contar_elementos(char **array) {
    int count = 0;
    while (array[count] != NULL) {
        count++;
    }
    return count;
}

void toLowerCase(char *str) {
    while (*str) {
        *str = tolower((unsigned char) *str);
        str++;
    }
}

bool contiene_palabra_clave(const char *cadena, const char *array[], int tam_array) {
    char *copia = strdup(cadena);
    char *token = strtok(copia, " ,.;()[]{}<>+-*/=!:\"\'\t\n\r"); // Delimitadores comunes
    while (token != NULL) {
        for (int i = 0; i < tam_array; i++) {
            if (strcmp(token, array[i]) == 0) {
                free(copia);
                return true;
            }
        }
        token = strtok(NULL, " ,.;()[]{}<>+-*/=!:\"\'\t\n\r");
    }
    free(copia);
    return false;
}

char **split_function(const char *str, int *count) {
    char *copia = strdup(str);
    char **resultado = (char **)malloc(2 * sizeof(char *));
    *count = 1;  // Inicialmente, asumimos que hay una sola parte

    char *pos = strrchr(copia, ':'); // Encuentra el último ':'
    
    if (pos == NULL) {
        resultado[0] = strdup(copia);
        resultado[1] = NULL;
    } else {
        *pos = '\0'; // Divide la cadena en dos partes
        resultado[0] = strdup(copia);
        resultado[1] = strdup(pos + 1);
        *count = 2;  // Ahora hay dos partes
    }

    free(copia);
    return resultado;
}

char *extraer_parentesis(const char *str) {
    const char *inicio = strchr(str, '('); // Encuentra el primer '('
    const char *fin = strrchr(str, ')');   // Encuentra el último ')'
    // ////printf("inicio: %s\n", inicio);
    // ////printf("fin: %s\n", fin);
    if (inicio == NULL || fin == NULL || inicio > fin) {
        return NULL; // Si no hay paréntesis o están en orden incorrecto
    }

    size_t len = fin - inicio - 1; // Longitud del contenido entre ()
    char *resultado = (char *)malloc(len + 1); // Reservar memoria
    if (resultado == NULL) {
        return NULL; // Manejo de error por falta de memoria
    }

    strncpy(resultado, inicio + 1, len); // Copiar contenido
    resultado[len] = '\0'; // Agregar el carácter nulo

    return resultado;
}

char *extraer_string(const char *str) {
    // Buscar las comillas simples y dobles
    const char *inicioComillasSimples = strchr(str, '\'');  // Encuentra la primera comilla simple
    const char *finComillasSimples = strrchr(str, '\'');    // Encuentra la última comilla simple

    const char *inicioComillasDobles = strchr(str, '\"');   // Encuentra la primera comilla doble
    const char *finComillasDobles = strrchr(str, '\"');     // Encuentra la última comilla doble

    // Comprobamos que las comillas simples o dobles existan y estén bien colocadas
    if (inicioComillasSimples && finComillasSimples && inicioComillasSimples < finComillasSimples) {
        size_t len = finComillasSimples - inicioComillasSimples - 1;
        char *resultado = (char *)malloc(len + 1);
        if (resultado) {
            strncpy(resultado, inicioComillasSimples + 1, len);
            resultado[len] = '\0';
            return resultado;
        }
    }

    if (inicioComillasDobles && finComillasDobles && inicioComillasDobles < finComillasDobles) {
        size_t len = finComillasDobles - inicioComillasDobles - 1;
        char *resultado = (char *)malloc(len + 1);
        if (resultado) {
            strncpy(resultado, inicioComillasDobles + 1, len);
            resultado[len] = '\0';
            return resultado;
        }
    }

    return NULL;  // Si no encuentra comillas válidas
}

char **split(const char *str, const char *delim, int *count) {
    char *copia = strdup(str); // Copia de la cadena original para no modificarla
    int capacidad = 10; // Tamaño inicial del array de resultados
    char **resultado = (char **)malloc(capacidad * sizeof(char *)); // Reservamos memoria
    int index = 0;

    // `strtok` divide la cadena en tokens usando el delimitador
    char *token = strtok(copia, delim);
    while (token != NULL) { 
        // Si se alcanza la capacidad, redimensionamos el array
        if (index >= capacidad) { 
            capacidad *= 2;
            resultado = (char **)realloc(resultado, capacidad * sizeof(char *));
        }
        resultado[index++] = strdup(token);  // Guardamos una copia del token

        token = strtok(NULL, delim); // Buscamos el siguiente token
    }

    resultado[index] = NULL;  // Marcar el final del array con NULL
    //////printf("index: %i\n", index);
    *count = index; // Guardamos el número de elementos en `count`

    free(copia);  // Liberamos la copia original

    return resultado; // Retornamos el array de punteros a strings
}

void mostrar_error(const char* mensaje, int linea, const char* detalle) {
    fprintf(stderr, "Error en la linea %d: %s -> %s\n", linea+2, mensaje, detalle);
    exit(1);
}

void mostrar_advertencia(const char* mensaje, int linea) {
    fprintf(stderr, "Advertencia en la linea %d: %s\n", linea, mensaje);
}

int es_tipo_valido(const char* tipo) {
    //////printf("TIPO: %s\n", tipo);
    for (int i = 0; i < sizeof(tipos_validos) / sizeof(tipos_validos[0]); i++) {
        //////printf("Estoy evaluando %s con %s (<- Este fue el que recibi por parametro)\n", tipos_validos[i], tipo);
        if (strcmp(tipo, tipos_validos[i]) == 0) {
            return true;  
        }
    }
    return false; 
}

void analizar_inicializacion_variables(Nodo* arbol, char* linea, int* num_linea, FILE* archivo, char* ultima_linea) {
    char buffer[256]; // Variable temporal para leer las líneas
    trim((char*)linea);
    Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
    agregar_hijo(arbol, nodo_keyword);
    int count = 0;
    while(fgets(buffer, sizeof(buffer), archivo)) {
        if (starts_with(buffer, "begin") || starts_with(buffer, "procedure") || starts_with(buffer, "function") || starts_with(buffer, "writeln")) {
            //strcpy(ultima_linea, buffer); // Copiar la última línea leída
            break;
        }
        (*num_linea)++;
        trim(buffer);
        if(buffer[0] == '\0'){
            continue;
        }
        if(!end_with_semicolon(buffer)){
            mostrar_error("; faltante", *num_linea, buffer);
        }
        trim_semicolon(buffer);
        char** partesInicializacion = split(buffer, ":", &count);
        Nodo* nodo_asignacion = crear_nodo("asignacion", buffer);
        agregar_hijo(nodo_keyword, nodo_asignacion);
        Nodo* nodo_variables = crear_nodo("variables", partesInicializacion[0]);
        agregar_hijo(nodo_asignacion, nodo_variables);
        char** partesVariableMismoTipo = split(partesInicializacion[0], ",", &count);
        int numVariablesMismoTipo = contar_elementos(partesVariableMismoTipo);
        if(numVariablesMismoTipo == 0){
            mostrar_error("No se han declarado variables", *num_linea, buffer);
        }
        if(numVariablesMismoTipo > 1){
            for(int i = 0; i < numVariablesMismoTipo; i++){
                trim(partesVariableMismoTipo[i]);
                Nodo* nodo_variable = crear_nodo("variable", partesVariableMismoTipo[i]);
                agregar_hijo(nodo_variables, nodo_variable);
                if(i < numVariablesMismoTipo - 1){
                    Nodo* nodo_coma = crear_nodo("coma", ",");
                    agregar_hijo(nodo_variables, nodo_coma);
                }
            }
        }

        trim(partesInicializacion[1]);
        toLowerCase(partesInicializacion[1]);
        int isValidType = es_tipo_valido(partesInicializacion[1]);
        if(!isValidType){
            mostrar_error("Tipo de dato no valido", *num_linea, buffer);
        }
        Nodo* nodo_dos_puntos = crear_nodo("dos_puntos", ":");
        agregar_hijo(nodo_asignacion, nodo_dos_puntos);
        Nodo* nodo_tipo = crear_nodo("tipo", partesInicializacion[1]);
        agregar_hijo(nodo_asignacion, nodo_tipo);
        
        for(int i = 0; i < count; i++){
            free(partesInicializacion[i]);
        }
        //printf("linea %i en analizar_inicializacion_variables: %s\n", *num_linea, buffer);
    }
    //printf("Linea al terminar el while en inicializacion variables: %i\n", *num_linea);
    
    strcpy(linea, buffer);
    //printf("Buffer %s o linea %s al terminar el while en inicializacion variables\n", buffer, linea);
}

void analizar_palabra_clave(Nodo* arbol, const char* linea, int *num_linea, bool fromF_Or_P, FILE* archivo) {
    char buffer[256]; // Variable temporal para leer las líneas
    trim((char*)linea);
    //printf("La linea que se supone es una palabra clave: %s\n", linea);
    const int num_palabras_clave = sizeof(palabras_clave) / sizeof(palabras_clave[0]);
    //printf("%i\n", strlen(linea));
    //printf("%i\n", strlen("begin"));
    //printf("Es un begin? %i\n", strcmp(linea, "begin") == 0);
    //printf("Vengo de una funcion or procedure? %i\n", fromF_Or_P);
    if((strcmp(linea, "begin") == 0) && fromF_Or_P){
        //printf("Es un begin dentro del cuerpo de una funcion o procedimiento\n");
        Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
        agregar_hijo(arbol, nodo_keyword);
        return;
    }else if((strcmp(linea, "begin") == 0) && !fromF_Or_P){
        //printf("Es un begin que no esta dentro del cuerpo de una funcion o procedimiento\n");
        Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
        agregar_hijo(arbol, nodo_keyword);
        while (fgets(buffer, sizeof(buffer), archivo)) {
            // Process the procedure body here
            (*num_linea)++;
            trim(buffer);
            if (buffer[0] == '\0') {
                continue;
            }
        //printf("linea %i en analizar_palabra_clave: %s\n", *num_linea, buffer);
        if(starts_with(buffer, "if")){
            //printf("Es un if dentro del begin que no esta dentro de una funcion\n");
            analizar_if(nodo_keyword, buffer, num_linea, archivo);
            
        }

        if(starts_with(buffer, "while")){
            //printf("linea %i en analizar_palabra_clave: %s\n", *num_linea, buffer);
            //printf("Es un while dentro del begin que no esta dentro de una funcion\n");
            analizar_while(nodo_keyword, buffer, num_linea, archivo);
        }

        if(starts_with(buffer, "writeln")){
            //printf("Es un writeln dentro del begin que no esta dentro de una funcion\n");
            analizar_writeln(nodo_keyword, buffer, *num_linea);
        }
        if(starts_with(buffer, "for")){
            //printf("Es un for dentro del begin que no esta dentro de una funcion\n");
            // char inicializacion[256];
            // char operador_control[256];
            // char final[256];
            // extraer_condicion_for(buffer, inicializacion, operador_control, final);
            // //printf("Incializacion: %s\n", inicializacion);
            // //printf("Operador de control: %s\n", operador_control);
            // //printf("Final: %s\n", final);
            analizar_for(nodo_keyword, buffer, num_linea, archivo);
        }
        if (starts_with(buffer, "end")) {
            Nodo* nodo_keyword = crear_nodo("palabra_clave", buffer);
            agregar_hijo(arbol, nodo_keyword);
            break;
        }
        }
        return;
    }else{
        Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
        agregar_hijo(arbol, nodo_keyword);
    }

}

void analizar_cabecera_funcion(Nodo* arbol, char* linea, int num_linea, char* nombre_funcion) {
    ////printf("ANALIZAR FUNCION CABECERA\n");
    obtenerNombreFuncion(linea, nombre_funcion, sizeof(nombre_funcion));
    char nombre_funcion_nosirve[256];
    int count = 0;
    //printf("linea %i en analizar cabecera funcion: %s\n", num_linea, linea);
    bool semicolon = end_with_semicolon(linea);
    //printf("Semicolon: %i\n", semicolon);
    if (strncmp(linea, "function ", 9) != 0) {
        mostrar_error("La declaración debe iniciar con 'function'", num_linea, linea);
    }

    char *abre_paren = strchr(linea, '(');
    char *cierra_paren = strchr(linea, ')');
    if (abre_paren == NULL || cierra_paren == NULL || cierra_paren < abre_paren) {
        mostrar_error("Error en la definición de los paréntesis", num_linea, linea);
    }

    char *dos_puntos = strchr(cierra_paren, ':');
    if (dos_puntos == NULL) {
        mostrar_error("Se esperaba ':' después de la lista de parámetros", num_linea, linea);
    }

    if (!semicolon) {
        mostrar_error("La declaración debe terminar con ';'", num_linea, linea);
    }

    Nodo* nodo_funcion = crear_nodo("funcion", nombre_funcion); // Create a node for the function
    agregar_hijo(arbol, nodo_funcion);
    Nodo *nodo_cabecera_funcion = crear_nodo("cabecera", ""); // Create a node for the function header
    agregar_hijo(nodo_funcion, nodo_cabecera_funcion);
    char **partes = split_function(linea, &count);
    trim(partes[0]);
    // ////printf("Header pt1 %s\n", partes[0]);
    Nodo *nodo_funcion1 = crear_nodo("header pt1", partes[0]); // Create a node for the first part of the header
    agregar_hijo(nodo_cabecera_funcion, nodo_funcion1);
    Nodo* nodo_dos_puntos = crear_nodo("dos puntos", ":"); // Create a node for the colon
    agregar_hijo(nodo_cabecera_funcion, nodo_dos_puntos);
    Nodo* nodo_funcion2 = crear_nodo("header pt2", partes[1]); // Create a node for the second part of the header
    agregar_hijo(nodo_cabecera_funcion, nodo_funcion2);
    // ////printf("Primera parte de la cadena de la funcion: %s\n", partes[0]);
    // ////printf("Segunda parte de la cadena de la funcion: %s\n", partes[1]);
    if (sscanf(partes[0], "function %[^;];", nombre_funcion_nosirve) == 1) {
        //////printf("Nombre de la funcion: %s\n", nombre_funcion);
        char *contenido_parentesis = extraer_parentesis(partes[0]);
        Nodo* nodo_parentesis1 = crear_nodo("parentesis", "'('");
        Nodo* nodo_parentesis2 = crear_nodo("parametros", contenido_parentesis);
        Nodo* nodo_parentesis3 = crear_nodo("parentesis", "')'");
        agregar_hijo(nodo_funcion1, nodo_parentesis1);
        agregar_hijo(nodo_funcion1, nodo_parentesis2);
        agregar_hijo(nodo_funcion1, nodo_parentesis3);
        //////printf("Contenido entre paréntesis: %s\n", contenido_parentesis);
        if (contenido_parentesis == NULL) {
            free(partes);
            mostrar_error("Funcion mal formada", num_linea, linea);
        }
        char **params = split(contenido_parentesis, ";", &count);
        int elem = contar_elementos(params);
        for (int i = 0; i < elem; i++) {
            Nodo* nodo_parametro = crear_nodo("parametro", params[i]);
            agregar_hijo(nodo_parentesis2, nodo_parametro);
            //////printf("Param %i: %s\n", i, params[i]);
            if (i < elem - 1) {
                ////printf("Agregra punto y coma ya que i es %i y elem es %i\n", i, elem);
                Nodo* nodo_coma = crear_nodo("punto y coma", ";");
                agregar_hijo(nodo_parentesis2, nodo_coma);
            }

            char **parametros = split(params[i], ":", &count);
            int count3 = contar_elementos(parametros);
            //////printf("Numero de elementos count3: %i\n", count3);
            char **paramsSameType = split(parametros[0], ",", &count);
            int numParamsSameType = contar_elementos(paramsSameType);
            //////printf("Numero de parametros del mismo tipo: %i\n", numParamsSameType);
            for (int j = 0; j < count3; j++) {
                //////printf("Parametro j %i: %s\n", j, parametros[j]);
                Nodo* nodo_param_inner = crear_nodo("parametro_inner", parametros[j]);
                agregar_hijo(nodo_parametro, nodo_param_inner);
                if (j < count3 - 1) {
                    Nodo* nodo_dos_puntos = crear_nodo("dos puntos", ":");
                    agregar_hijo(nodo_parametro, nodo_dos_puntos);
                    if (numParamsSameType > 1) {
                        for (int k = 0; k < numParamsSameType; k++) {
                            Nodo* nodo_param_inner2 = crear_nodo("parametro_inner", paramsSameType[k]);
                            agregar_hijo(nodo_param_inner, nodo_param_inner2);
                            if (k < numParamsSameType - 1) {
                                Nodo* nodo_coma = crear_nodo("coma", ",");
                                agregar_hijo(nodo_param_inner, nodo_coma);
                            }
                        }
                    }
                }
                trim(parametros[1]);
                toLowerCase(parametros[1]);
                int chars = strlen(parametros[1]);
                //////printf("%s tiene %i letras\n", parametros[1], chars);

                int isValidType = es_tipo_valido(parametros[1]);
                //////printf("isValidType: %i\n", isValidType);
                if (!isValidType) {
                    mostrar_error("Tipo de dato no valido", num_linea, linea);
                }
            }
            trim(partes[1]);
            trim_semicolon(partes[1]);
            toLowerCase(partes[1]);
            int isValidType = es_tipo_valido(partes[1]);
            //////printf("isValidReturnType: %i\n", isValidType);
            if (!isValidType) {
                mostrar_error("Tipo de retorno de la funcion dato no valido", num_linea, linea);
            }
            free(parametros);
        }
        free(partes);
        free(contenido_parentesis);
        free(params);
    } else {
        mostrar_error("Expresion ilegal", num_linea, linea);
    }

}

void analizar_funcion(Nodo* arbol, char* linea, int* num_linea, FILE* archivo, char* nombre_funcion) {
    char buffer[256]; // Variable temporal para leer las líneas
    bool cabecera_analizada = false; // Variable de control para la cabecera
    //char nombre_funcion[50];
    int count = 0;
    ////printf("ANALIZAR FUNCION\n");
    if(!cabecera_analizada) {
        analizar_cabecera_funcion(arbol, linea, *num_linea, nombre_funcion);
        cabecera_analizada = true;
    }
    ////printf("Nombre de la funcion: %s\n", nombre_funcion);
    Nodo* nodo_cuerpo_funcion = crear_nodo("cuerpo_funcion", "");
    agregar_hijo(arbol, nodo_cuerpo_funcion);
    

    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        trim(buffer);
        ////printf("linea en analizar_funcion con linea %i: %s\n", *num_linea, linea);
        if(buffer[0] == '\0'){
            continue;
        }

        //printf("linea en analizar_funcion con buffer %i: %s\n", *num_linea, buffer);

        //////printf("a");
        //////printf("La cadena contiene := %s\n", strstr(buffer, ":="));
        if(strstr(buffer, ":=") != NULL){
            ////printf("Es una asignacion\n");
            char** partesRetornoFuncion = split(buffer, ":=", &count);
            trim(partesRetornoFuncion[0]);
            trim(partesRetornoFuncion[1]);

            toLowerCase(partesRetornoFuncion[0]);
            toLowerCase(nombre_funcion);
            if(partesRetornoFuncion[0][0] == '\0'){
                mostrar_error("No se ha asignado una variable al valor de retorno", *num_linea, buffer);
            }else if(partesRetornoFuncion[1][0] == '\0'){
                mostrar_error("No se ha asignado un valor a la variable de retorno", *num_linea, buffer);
            }else if(strcmp(partesRetornoFuncion[0], nombre_funcion) != 0){
                //printf("PartesRetornoFuncion[0]: %s\n", partesRetornoFuncion[0]);
                //printf("Nombre de la funcion: %s\n", nombre_funcion);
                mostrar_error("La variable de retorno no coincide con el nombre de la funcion", *num_linea, buffer);
            }
            ////printf("Parte 0 de la asignacion dentro de la funcion: %s\n", partesRetornoFuncion[0]);
            ////printf("Parte 1 de la asignacion dentro de la funcion: %s\n", partesRetornoFuncion[1]);
            analizar_asignacion(nodo_cuerpo_funcion, buffer, *num_linea);
        }

        if(contiene_elemento(buffer, palabras_clave, sizeof(palabras_clave) / sizeof(palabras_clave[0]))){
            analizar_palabra_clave(nodo_cuerpo_funcion, buffer, num_linea, true, archivo);{
        }
 


        //////printf("cabecera analizada: %s\n", cabecera_analizada ? "true" : "false");

        if (strcmp(buffer, "end;") == 0) {
            //printf("Es un end, significa el final de la funcion\n");
            break;
            }
        }
    }

    //free(nombre_funcion);
    // free(buffer);
    // free(linea);
    // free()
}

void analizar_expresion(Nodo* arbol, char* expr, int num_linea) {
    ////printf("ANALIZAR EXPRESION\n");
    removeSpaces(expr);
    trim_semicolon(expr);
    ////printf("EXPRESION: %s\n", expr);
    
    int count = strlen(expr);
    ////printf("COUNT: %i\n", count);
    int num_operadores = sizeof(operadoresAritmeticos) / sizeof(operadoresAritmeticos[0]);
    ////printf("NUM OPERADORES: %i\n", num_operadores);

    for (int i = 0; i < count; i++) {
        ////printf("CHAR: %c\n", expr[i]);
        bool es_operador = false;

        // Verificar si el carácter es un operador
        for (int j = 0; j < num_operadores; j++) {  // Suponiendo que num_operadores es la cantidad real de operadoresAritmeticos
            int op_len = strlen(operadoresAritmeticos[j]);

            if (i + op_len <= count && strncmp(&expr[i], operadoresAritmeticos[j], op_len) == 0) {
                ////printf("OPERADOR: %s\n", operadoresAritmeticos[j]);
                Nodo* nodo_operador = crear_nodo("operador", operadoresAritmeticos[j]);
                agregar_hijo(arbol, nodo_operador);
                es_operador = true;
                i += op_len - 1; // Saltar operador completo
                break;
            }
        }

        if (!es_operador) {
            char operando[2] = {expr[i], '\0'};
            ////printf("OPERANDO: %s\n", operando);
            Nodo* nodo_operando = crear_nodo("operando", operando);
            agregar_hijo(arbol, nodo_operando);
        }
    }
}

void analizar_asignacion(Nodo* arbol, const char* linea, int num_linea){
    printf("ANALIZAR ASIGNACION\n");
    printf("linea: %s\n", linea);

    int count = 0;
    char** partes = split(linea, ":=", &count);
    printf("Count: %i\n", count);
    printf("Partes 0 %s\n", partes[0]);
    printf("Partes 1 %s\n", partes[1]);
    
    if (!partes || count != 2) {
        mostrar_error("Asignacion mal formada", num_linea, linea);
        return;
    }

    trim(partes[0]);
    trim(partes[1]);
    toLowerCase(partes[0]);
    toLowerCase(partes[1]);

    ////printf("Parte 1: %s\n", partes[0]);
    ////printf("Parte 2: %s\n", partes[1]);

    Nodo* nodo_asignacion = crear_nodo("asignacion", "");
    agregar_hijo(arbol, nodo_asignacion);
    
    Nodo* nodo_variable = crear_nodo("variable", partes[0]);
    Nodo* nodo_asignacion_operador = crear_nodo("asignacion_operador", ":=");
    Nodo* nodo_expresion = crear_nodo("expresion", partes[1]);

    agregar_hijo(nodo_asignacion, nodo_variable);
    agregar_hijo(nodo_asignacion, nodo_asignacion_operador);
    agregar_hijo(nodo_asignacion, nodo_expresion);
    if(strlen(partes[1]) > 1){
        analizar_expresion(nodo_expresion, partes[1], num_linea);
    }
    //analizar_expresion(nodo_expresion, partes[1], num_linea);

    // Liberar memoria correctamente
    for (int i = 0; i < count; i++) {
        free(partes[i]);
    }
    free(partes);
}

void analizar_procedure(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo, char* nombre_procedure) {
    char buffer[256]; // Variable temporal para leer las líneas
    //printf("ANALIZAR PROCEDURE\n");
    trim((char*)linea);
    obtenerNombreProcedure(linea, nombre_procedure, sizeof(nombre_procedure));
    //printf("Nombre del procedure: %s\n", nombre_procedure);

    if (strncmp(linea, "procedure ", 10) != 0) {
        mostrar_error("La declaración debe iniciar con 'procedure'", *num_linea, linea);
    }

    if (!end_with_semicolon(linea)) {
        mostrar_error("La declaración debe terminar con ';'", *num_linea, linea);
    }

    Nodo* nodo_procedure = crear_nodo("procedure", nombre_procedure);
    agregar_hijo(arbol, nodo_procedure);

    while (fgets(buffer, sizeof(buffer), archivo)) {
        // Process the procedure body here
        (*num_linea)++;
        trim(buffer);
        
        if (buffer[0] == '\0') {
            
            continue;
        }
        //printf("Buffer o linea en analizar_procedure: %s\n", buffer);
        if(contiene_palabra_clave(buffer, palabras_clave, sizeof(palabras_clave) / sizeof(palabras_clave[0]))){
            //printf("Encontre una palabra clave\n");
            analizar_palabra_clave(nodo_procedure, buffer, num_linea, true, archivo);
        }

        if(strstr(buffer, "writeln") != NULL){
            //printf("Encontre un writeln\n");
            analizar_writeln(nodo_procedure, buffer, *num_linea);
        }

        if(strcmp(buffer, "end;") == 0){
            //printf("Es un end el procedure\n");
            break;
        }
    }
}

void analizar_writeln(Nodo* arbol, const char* linea, int num_linea) {
    //printf("ANALIZAR WRITELN\n");
    char contenido[256];
    contenidoWriteln((char*)linea, contenido);
    trim((char*)linea);
    if (!end_with_semicolon(linea)) {
        mostrar_error("La declaración debe terminar con ';'", num_linea, linea);
    }
    //printf("Contenido del writeln: %s\n", contenido);
    Nodo* nodo_writeln = crear_nodo("writeln", linea);
    agregar_hijo(arbol, nodo_writeln);
    char* contenido_en_parentesis = extraer_parentesis(linea);
    if((starts_with(contenido_en_parentesis, "\'"))){
        printf("%s empieza con comillas simples\n", contenido_en_parentesis);
        if(!ends_with(contenido_en_parentesis, "\'")){
            printf("no termina con comillas simples\n");
            mostrar_error("Comilla simple faltante", num_linea, linea);
        }
    }
    if(ends_with(contenido_en_parentesis, "\'")){
        printf("%s termina con comillas simples\n", contenido_en_parentesis);
        if(!starts_with(contenido_en_parentesis, "\'")){
            printf("no empieza con comillas simples\n");
            //printf("%s", contenido_en_parentesis[1]);
            mostrar_error("Comilla simple faltante", num_linea, linea);
        }
    }

    size_t len = strlen(contenido_en_parentesis);

    // if (len > 0) {
    //     if ((contenido[0] == '\'' && contenido[len - 1] != '\'') &&
    //       (contenido[0] != '\'' && contenido[len - 1] == '\'')) {
    //       mostrar_error("Debe comenzar por comillas simples y viceversa", num_linea, linea);
    //     }
    // }
    
    if (contenido[0] != '\'' ||  contenido[len - 1] != '\'') {
          int espacio_en_medio = 0;
          for (size_t i = 1; i < len - 1; i++) { 
              if (isspace(contenido[i])) {
                  espacio_en_medio = 1;
                  break;
              }
          }
          if (espacio_en_medio) {
              mostrar_error("El contenido del writeln no puede contener espacios en medio de los caracteres a menos que este encerrado entre comillas simples", num_linea, linea);
          }
      }

    if(!starts_with(contenido_en_parentesis, "\'") && !ends_with(contenido_en_parentesis, "\'")){
        // for (int i = 0; i < con; i++)
        // {
        //     /* code */
        // }
        
    }
    //printf("Contenido en parentesis: %s\n", contenido_en_parentesis);
    Nodo* contenido_writeln = crear_nodo("contenido", contenido_en_parentesis);
    agregar_hijo(nodo_writeln, contenido_writeln);
    
}

void analizar_if(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo) {
    //printf("ANALIZAR IF\n");
    char condicion[256];
    char buffer[256];
    extraer_condicion_if(linea, condicion);
    trim((char*)linea);
    // if (!end_with_semicolon(linea)) {
    //     mostrar_error("La declaración debe terminar con ';'", *num_linea, linea);
    // }
    //printf("Condicion del if: %s\n", condicion);
    int terminaConThen = ends_with(linea, "then");
    printf("Termina con then: %i\n", terminaConThen);
    if(!terminaConThen){
        mostrar_error("La estructura if debe terminar con 'then'", *num_linea, linea);
    }
    Nodo* nodo_if_statement = crear_nodo("if_statement", linea);
    agregar_hijo(arbol, nodo_if_statement);
    Nodo* nodo_if = crear_nodo("if", "if");
    agregar_hijo(nodo_if_statement, nodo_if);
    // char* contenido_en_parentesis = extraer_parentesis(linea);
    // //printf("Contenido en parentesis: %s\n", contenido_en_parentesis);
    Nodo* contenido_if = crear_nodo("contenido", condicion);
    agregar_hijo(nodo_if, contenido_if);
    int num_operadores = sizeof(operadoresDeComparacion) / sizeof(operadoresDeComparacion[0]);
    int count = 0;
    //printf("Numero de operadores de comparacion: %i\n", num_operadores);
    for(int i = 0; i < sizeof(operadoresDeComparacion)/sizeof(operadoresDeComparacion[0]); i++){
        //printf("Operador de comparacion: %s\n", operadoresDeComparacion[i]);
        if(strstr(condicion, operadoresDeComparacion[i]) != NULL){
            //printf("Encontre un operador de comparacion, que es: %s\n", operadoresDeComparacion[i]);
            char** partesCondicion = split(condicion, operadoresDeComparacion[i], &count);
            //printf("Parte 1 de la condicion: %s\n", partesCondicion[0]);
            //printf("Parte 2 de la condicion: %s\n", partesCondicion[1]);
            Nodo* nodo_operador_izq = crear_nodo("nodo_operador_izq", partesCondicion[0]);
            Nodo* nodo_operador_der = crear_nodo("nodo_operador_der", partesCondicion[1]);
            Nodo* nodo_operador = crear_nodo("operador", operadoresDeComparacion[i]);
            agregar_hijo(contenido_if, nodo_operador_izq);
            agregar_hijo(contenido_if, nodo_operador);
            agregar_hijo(contenido_if, nodo_operador_der);
            break;
        }
    }
    Nodo* then = crear_nodo("then", "then");
    agregar_hijo(nodo_if_statement, then);
    while(fgets(buffer, sizeof(buffer), archivo)){
        (*num_linea)++;
        trim((char*)buffer);
        if(linea[0] == '\0'){
            continue;
        }
        //printf("linea %i en analizar_if: %s\n", *num_linea, buffer);
        if(starts_with(buffer, "while") || starts_with(buffer, "for")){
            //printf("Termino el if y dio comienzo otra estructura\n");
            strcpy((char*)linea, buffer);
            break;
        }
        Nodo* nodo_sentencia = crear_nodo("sentencia", "");
        agregar_hijo(nodo_if_statement, nodo_sentencia);
        if(strstr(buffer, "writeln") != NULL){
            //printf("Encontre un writeln en el if");
            analizar_writeln(nodo_sentencia, buffer, *num_linea);
        }
        //if()
    }
    //analizar_palabra_clave(nodo_if_statement, then, num_linea, false, archivo);
    //agregar_hijo(nodo_if, contenido_if);
}

void analizar_while(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo){
    //printf("ANALIZAR WHILE\n");
    char condicion[256];
    char buffer[256];
    trim((char*)linea);
    extraer_condicion_while(linea, condicion);
    int terminaConDo = ends_with(linea, "do");
    printf("Termina con do: %i\n", terminaConDo);
    if(!terminaConDo){
        mostrar_error("La estructura while debe terminar con 'do'", *num_linea, linea);
    }
    //printf("Condicion del while: %s\n", condicion);
    Nodo* while_statement = crear_nodo("while_statement", linea);
    agregar_hijo(arbol, while_statement);
    Nodo* nodo_while = crear_nodo("while", "while");
    agregar_hijo(while_statement, nodo_while);
    trim(condicion);
    Nodo* contenido_while = crear_nodo("contenido", condicion);
    agregar_hijo(nodo_while, contenido_while);
    int num_operadores = sizeof(operadoresDeComparacion) / sizeof(operadoresDeComparacion[0]);
    int count = 0;
    //printf("Numero de operadores de comparacion: %i\n", num_operadores);
    for(int i = 0; i < num_operadores; i++){
        //printf("Operador de comparacion: %s\n", operadoresDeComparacion[i]);
        if(strstr(condicion, operadoresDeComparacion[i]) != NULL){
            //printf("Encontre un operador de comparacion, que es: %s\n", operadoresDeComparacion[i]);
            char** partesCondicion = split(condicion, operadoresDeComparacion[i], &count);
            //printf("Parte 1 de la condicion: %s\n", partesCondicion[0]);
            //printf("Parte 2 de la condicion: %s\n", partesCondicion[1]);
            Nodo* nodo_operador_izq = crear_nodo("nodo_operador_izq", partesCondicion[0]);
            Nodo* nodo_operador_der = crear_nodo("nodo_operador_der", partesCondicion[1]);
            Nodo* nodo_operador = crear_nodo("operador", operadoresDeComparacion[i]);
            agregar_hijo(contenido_while, nodo_operador_izq);
            agregar_hijo(contenido_while, nodo_operador);
            agregar_hijo(contenido_while, nodo_operador_der);
            break;
        }
    }

    Nodo* nodo_do = crear_nodo("do", "do");
    agregar_hijo(while_statement, nodo_do);
    while(fgets(buffer, sizeof(buffer), archivo)){
        (*num_linea)++;
        trim((char*)buffer);
        if(linea[0] == '\0'){
            continue;
        }
        //printf("linea %i en analizar_if: %s\n", *num_linea, buffer);
        if(starts_with(buffer, "if") || starts_with(buffer, "for") || starts_with(buffer, "while")){
            //printf("Termino el while y dio comienzo otra estructura\n");
            strcpy((char*)linea, buffer);
            break;
        }
        Nodo* nodo_sentencia = crear_nodo("sentencia", "");
        agregar_hijo(while_statement, nodo_sentencia);
        if(strstr(buffer, "writeln") != NULL){
            //printf("Encontre un writeln en el if");
            analizar_writeln(nodo_sentencia, buffer, *num_linea);
        }
        //if()
    }

}

void analizar_for(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo){
    //printf("ANALIZAR FOR\n");
    //char condicion[256];
    char buffer[256];
    trim((char*)linea);
    char inicializacion[256];
    char operador_control[256];
    char final[256];
    int count = 0;
    int terminaConDo = ends_with(linea, "do");
    printf("Termina con do: %i\n", terminaConDo);
    if(!terminaConDo){
        mostrar_error("La estructura for debe terminar con 'do'", *num_linea, linea);
    }
    extraer_condicion_for(linea, inicializacion, operador_control, final);
    //printf()
    ////printf("Condicion del for: %s\n", condicion);
    printf("Incializacion del for: %s\n", inicializacion);
    char** partesInicializacion = split(inicializacion, ":=", &count);
    //printf("Parte 1 de la inicializacion: %s\n", partesInicializacion[0]);
    //printf("Parte 2 de la inicializacion: %s\n", partesInicializacion[1]);
    
    //printf("Operador de control del for: %s\n", operador_control);
    //printf("Final del for: %s\n", final);
    Nodo* for_statement = crear_nodo("for_statement", linea);
    agregar_hijo(arbol, for_statement);
    Nodo* nodo_for = crear_nodo("for", "for");
    agregar_hijo(for_statement, nodo_for);
    if(inicializacion[0]=='\0'){
        mostrar_error("Operador de control to o downto no encontrado", *num_linea, linea);
    }
    analizar_asignacion(for_statement, inicializacion, *num_linea);
    //Nodo* nodo_inicializacion =
    Nodo *nodo_operador_control = crear_nodo("operador_control", operador_control);
    agregar_hijo(for_statement, nodo_operador_control);
    Nodo *nodo_final = crear_nodo("final", final);
    agregar_hijo(for_statement, nodo_final);
    Nodo* nodo_do = crear_nodo("do", "do");
    agregar_hijo(for_statement, nodo_do);
    while(fgets(buffer, sizeof(buffer), archivo)){
        (*num_linea)++;
        trim((char*)buffer);
        if(inicializacion[0] == '\0'){
            mostrar_error("No se ha inicializado la variable de control del for", *num_linea, buffer);
            continue;
        }
        if(linea[0] == '\0'){
            ////printf("Linea vacia, no hay :=\n");
            continue;
        }
        //printf("linea %i en analizar_for: %s\n", *num_linea, buffer);
        if(starts_with(buffer, "if") || starts_with(buffer, "for") || starts_with(buffer, "while")){
            //printf("Termino el for y dio comienzo otra estructura\n");
            strcpy((char*)linea, buffer);
            break;
        }

        if(starts_with(buffer, "end")){
            //printf("Es un if dentro del for\n");
            analizar_palabra_clave(arbol, buffer, num_linea, false, archivo);
            break;
        }
        Nodo* nodo_sentencia = crear_nodo("sentencia", "");
        agregar_hijo(for_statement, nodo_sentencia);
        if(strstr(buffer, "writeln") != NULL){
            //printf("Encontre un writeln en el for\n");
            analizar_writeln(nodo_sentencia, buffer, *num_linea);
        }
        //if()
     }
    }


void imprimir_arbol(Nodo* nodo, int nivel) {
    for (int i = 0; i < nivel; i++) printf("  ");  
    printf("%s(%s)\n", nodo->tipo, nodo->valor);  

    for (int i = 0; i < nodo->num_hijos; i++) {
        imprimir_arbol(nodo->hijos[i], nivel + 1); 
    }
}

int main() {
    FILE* archivo = fopen("codigo_pascal.txt", "r");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return 1; // Return an error code
    }

    const char *text = "Sumar := a + b + c * d;";

    Nodo* arbol = crear_nodo("programaPrueba", "");
    //analizar_asignacion(arbol, text, 1);

    char linea[256];
    char nombre_funcion[50];
    char nombre_procedure[50];  
    int num_linea = 0;
    while (fgets(linea, sizeof(linea), archivo)) {
        trim(linea);
        char ultima_linea[256];
        ////printf("Ultima linea: %s\n", ultima_linea);
        if (*linea == '\0') {
            num_linea++;
            continue;
        };  // Ignorar lineas vacias
        for (int i = 0; linea[i]; i++) linea[i] = tolower(linea[i]);

        if(starts_with(linea, "var")) {

            analizar_inicializacion_variables(arbol, linea, &num_linea, archivo, ultima_linea);

        }
        //printf("linea en main2 %i: %s\n", num_linea, linea);
        if (starts_with(linea, "function")) {
            //printf("Es una funcion en linea %i, %s\n", num_linea, linea);
            analizar_funcion(arbol, linea, &num_linea, archivo, nombre_funcion);
        }else if(starts_with(linea, "procedure")){
            //printf("Es un procedure en linea %i, %s\n", num_linea, linea);
            analizar_procedure(arbol, linea, &num_linea, archivo, nombre_procedure);
        }else if(starts_with(linea, "if")){
            //printf("Es un if en linea %i, %s\n", num_linea, linea);
            char condicion[256];
            extraer_condicion_if(linea, condicion);
            //printf("Condicion del if: %s\n", condicion);
        }else if(starts_with(linea, "begin")){
            //printf("Es un begin en linea %i, %s\n", num_linea, linea);
            analizar_palabra_clave(arbol, linea, &num_linea, false, archivo);
        }else if(starts_with(linea, "while")){
            //printf("Es un while en linea %i, %s\n", num_linea, linea);
            analizar_while(arbol, linea, &num_linea, archivo);
        }else if(starts_with(linea, "for")){
            //printf("Es un for en linea %i, %s\n", num_linea, linea);
        }else if(starts_with(linea, "writeln")){
            //printf("Es un writeln en linea %i, %s\n", num_linea, linea);
            analizar_writeln(arbol, linea, num_linea);
        }
        num_linea++;
        //printf("Incrementando num_linea: %i\n", num_linea);
        //////printf("linea %i: %s\n", num_linea, linea);
    }

    fclose(archivo);
    imprimir_arbol(arbol, 0);

    return 0; // Return success code
}