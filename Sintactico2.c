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

bool es_palabra_clave_similar(const char* palabra, int num_linea);
bool validar_asignacion(const char* linea, int num_linea);
bool validar_condicion(const char* condicion, int num_linea);
bool validar_parametros_funcion(const char* parametros, int num_linea);

bool es_palabra_clave_similar(const char* palabra, int num_linea) {
    char palabra_sin_puntuacion[256];
    strcpy(palabra_sin_puntuacion, palabra);
  
    int len = strlen(palabra_sin_puntuacion);
    if (len > 0 && (palabra_sin_puntuacion[len-1] == '.' || palabra_sin_puntuacion[len-1] == ';')) {
        palabra_sin_puntuacion[len-1] = '\0';
    }
    
    for (int i = 0; i < sizeof(palabras_clave) / sizeof(palabras_clave[0]); i++) {
        if (strcmp(palabra_sin_puntuacion, palabras_clave[i]) == 0) {
            return false; 
        }
    }

    if (strcmp(palabra_sin_puntuacion, "en") == 0) {
        mostrar_error("Palabra clave incorrecta: 'en' (¿quiso escribir 'end'?)", num_linea, palabra);
        return true;
    }
    
    // Check for typos
    for (int i = 0; i < sizeof(palabras_clave) / sizeof(palabras_clave[0]); i++) {
        int len1 = strlen(palabra_sin_puntuacion);
        int len2 = strlen(palabras_clave[i]);
        
        if (abs(len1 - len2) <= 1) {
            int differences = 0;
            int minLen = (len1 < len2) ? len1 : len2;
            
            for (int j = 0; j < minLen && differences <= 2; j++) {
                if (palabra_sin_puntuacion[j] != palabras_clave[i][j]) {
                    differences++;
                }
            }
            differences += abs(len1 - len2);
            
            if (differences <= 2) {
                char mensaje[100];
                sprintf(mensaje, "Posible error tipográfico: '%s' (¿quiso escribir '%s'?)", 
                        palabra_sin_puntuacion, palabras_clave[i]);
                mostrar_error(mensaje, num_linea, palabra);
                return true;
            }
        }
    }
    
    return false;
}

bool validar_asignacion(const char* linea, int num_linea) {
    if (strstr(linea, "=") != NULL && strstr(linea, ":=") == NULL) {
        mostrar_error("Operador de asignación inválido. Debe usar ':='", num_linea, linea);
        return false;
    }

    char* dos_puntos = strstr(linea, ":");
    char* igual = strstr(linea, "=");
    if ((dos_puntos && !igual) || (!dos_puntos && igual)) {
        mostrar_error("Operador de asignación mal formado. Debe ser ':='", num_linea, linea);
        return false;
    }
    if (strstr(linea, ":=") != NULL) {
        int count = 0;
        char** partes = split(linea, ":=", &count);
        if (!partes || count != 2 || !partes[0] || !partes[1] || 
            strlen(partes[0]) == 0 || strlen(partes[1]) == 0) {
            if (partes) {
                for(int i = 0; i < count; i++) {
                    free(partes[i]);
                }
                free(partes);
            }
            mostrar_error("Asignación mal formada", num_linea, linea);
            return false;
        }
        for(int i = 0; i < count; i++) {
            free(partes[i]);
        }
        free(partes);
    }
    return true;
}

bool validar_condicion(const char* condicion, int num_linea) {
    bool operador_encontrado = false;
    for (int i = 0; i < sizeof(operadoresDeComparacion)/sizeof(operadoresDeComparacion[0]); i++) {
        if (strstr(condicion, operadoresDeComparacion[i]) != NULL) {
            operador_encontrado = true;
            if (strstr(condicion, "''") != NULL) {
                return true;
            }
            break;
        }
    }
    
    if (!operador_encontrado) {
        mostrar_error("Condición inválida: falta operador de comparación válido", num_linea, condicion);
        return false;
    }
    return true;
}

bool validar_parametros_funcion(const char* parametros, int num_linea) {
    char* copia = strdup(parametros);
    char* token = strtok(copia, ":");
    
    while (token != NULL) {
        char* params = strchr(token, ',');
        if (params != NULL) {
            // Verify there's content before the comma
            char* antes = token;
            while (isspace(*antes)) antes++;
            if (*antes == ',') {
                mostrar_error("Error en la lista de parametros: falta variable antes de la coma", num_linea, parametros);
                free(copia);
                return false;
            }

            // Verify there's content after the comma
            char* despues = params + 1;
            while (*despues && isspace(*despues)) despues++;
            if (*despues == '\0' || *despues == ':') {
                mostrar_error("Error en la lista de parametros: falta variable después de la coma", num_linea, parametros);
                free(copia);
                return false;
            }
        }
        token = strtok(NULL, ":");
    }
    free(copia);
    return true;
}

Nodo* crear_nodo(const char* tipo, const char* valor) {
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
        inicio_while += 5;
        while (*inicio_while && isspace(*inicio_while)) {
            inicio_while++;
        }

        const char* fin_do = strstr(inicio_while, "do");
        if (fin_do != NULL) {
            size_t longitud = fin_do - inicio_while;
            strncpy(condicion, inicio_while, longitud);
            condicion[longitud] = '\0';  
        } else {
            condicion[0] = '\0';  
        }
    } else {
        condicion[0] = '\0';  
    }
}

void obtenerNombreFuncion(const char *cabecera, char *nombre, size_t tam) {
    const char *ptr = cabecera;
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }
    
   const char *palabra = "function";
    size_t len = strlen(palabra);
    
    if (strncasecmp(ptr, palabra, len) != 0) {
        nombre[0] = '\0';
        return;
    }
    
    ptr += len;
    
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }
    
    size_t i = 0;
    while (*ptr && !isspace(*ptr) && *ptr != '(' && i < tam - 1) {
        nombre[i++] = *ptr;
        ptr++;
    }
    nombre[i] = '\0';
    }

void obtenerNombreProcedure(const char *cabecera, char *nombre, size_t tam) {
    const char *ptr = cabecera;
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }
    
    const char *palabra = "procedure";
    size_t len = strlen(palabra);
    
    if (strncasecmp(ptr, palabra, len) != 0) {
        nombre[0] = '\0';
        return;
    }
    
    ptr += len;
    
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }
    
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
    const char* inicio_if = strstr(linea, "if");  
    if (inicio_if != NULL) {
        inicio_if += 2;

        while (*inicio_if && isspace(*inicio_if)) {
            inicio_if++;
        }

        const char* fin_then = strstr(inicio_if, "then");
        if (fin_then != NULL) {
            size_t longitud = fin_then - inicio_if;
            strncpy(condicion, inicio_if, longitud);
            condicion[longitud] = '\0';  
        } else {
            condicion[0] = '\0';  
        }
    } else {
        condicion[0] = '\0'; 
    }
}

void extraer_condicion_for(const char* linea, char* inicializacion, char* operador_control, char* final) {
    const char* inicio_for = strstr(linea, "for");  
    if (inicio_for != NULL) {
        inicio_for += 3;

        while (*inicio_for && isspace(*inicio_for)) {
            inicio_for++;
        }

        const char* asignacion = strstr(inicio_for, ":=");
        if (asignacion != NULL) {
            asignacion += 2; 
            
            while (*asignacion && isspace(*asignacion)) {
                asignacion++;
            }

            const char* fin_inicializacion = strstr(asignacion, " to");
            if (fin_inicializacion == NULL) {
                fin_inicializacion = strstr(asignacion, " downto");
            }

            if (fin_inicializacion != NULL) {
                size_t longitud = fin_inicializacion - inicio_for;
                strncpy(inicializacion, inicio_for, longitud);
                inicializacion[longitud] = '\0';  
                inicio_for = fin_inicializacion; 
            } else {
                inicializacion[0] = '\0';
                return;
            }
        } else {
            inicializacion[0] = '\0';
            return;
        }

        while (*inicio_for && isspace(*inicio_for)) {
            inicio_for++;
        }

        if (strncmp(inicio_for, "to", 2) == 0) {
            strcpy(operador_control, "to");
            inicio_for += 2;  
        } else if (strncmp(inicio_for, "downto", 6) == 0) {
            strcpy(operador_control, "downto");
            inicio_for += 6;  
        } else {
            operador_control[0] = '\0';
            return;
        }

        while (*inicio_for && isspace(*inicio_for)) {
            inicio_for++;
        }

        const char* fin_final = strstr(inicio_for, " do");
        if (fin_final != NULL) {
            size_t longitud = fin_final - inicio_for;
            strncpy(final, inicio_for, longitud);
            final[longitud] = '\0';  
        } else {
            final[0] = '\0';
        }
    } else {
        inicializacion[0] = '\0';
        operador_control[0] = '\0';
        final[0] = '\0';
    }
}

bool end_with_semicolon(const char* str) {
    trim((char*)str);
    size_t len = strlen(str);
    if (str[len - 1] != ';') {
        return false;
    }
    return true;
}

void removeSpaces(char *str) {
    char *dst = str; 
    while (*str) {
        if (!isspace((unsigned char)*str)) { 
            *dst++ = *str; 
        }
        str++;
    }
    *dst = '\0'; 
}

void trim(char *str) {
    char *start = str;
    char *end;

    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *str = '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

bool contiene_elemento(const char *cadena, const char *array[], int tam_array) {
    for (int i = 0; i < tam_array; i++) {
        if (strstr(cadena, array[i]) != NULL) { 
            return true;  
			}
    }
    return false;  
}

void trim_semicolon(char *str) {
    int len = strlen(str);

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
    char *token = strtok(copia, " ,.;()[]{}<>+-*/=!:\"\'\t\n\r"); 
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
    *count = 1;  

    char *pos = strrchr(copia, ':'); 
    
    if (pos == NULL) {
        resultado[0] = strdup(copia);
        resultado[1] = NULL;
    } else {
        *pos = '\0'; 
        resultado[0] = strdup(copia);
        resultado[1] = strdup(pos + 1);
        *count = 2;  
    }

    free(copia);
    return resultado;
}

char *extraer_parentesis(const char *str) {
    const char *inicio = strchr(str, '('); 
    const char *fin = strrchr(str, ')');   
    if (inicio == NULL || fin == NULL || inicio > fin) {
        return NULL; 
    }

    size_t len = fin - inicio - 1; 
    char *resultado = (char *)malloc(len + 1); 
    if (resultado == NULL) {
        return NULL; 
    }

    strncpy(resultado, inicio + 1, len); 
    resultado[len] = '\0';

    return resultado;
}

char *extraer_string(const char *str) {
    const char *inicioComillasSimples = strchr(str, '\'');  
    const char *finComillasSimples = strrchr(str, '\'');    

    const char *inicioComillasDobles = strchr(str, '\"');   
    const char *finComillasDobles = strrchr(str, '\"');     

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

    return NULL;  
}

char **split(const char *str, const char *delim, int *count) {
    char *copia = strdup(str); 
    int capacidad = 10; 
    char **resultado = (char **)malloc(capacidad * sizeof(char *)); 
    int index = 0;

    char *token = strtok(copia, delim);
    while (token != NULL) { 
        if (index >= capacidad) { 
            capacidad *= 2;
            resultado = (char **)realloc(resultado, capacidad * sizeof(char *));
        }
        resultado[index++] = strdup(token);  

        token = strtok(NULL, delim); 
    }

    resultado[index] = NULL;  
     *count = index; 

    free(copia); 

    return resultado; 
}

void mostrar_error(const char* mensaje, int linea, const char* detalle) {
    fprintf(stderr, "Error en la linea %d: %s -> %s\n", linea+2, mensaje, detalle);
    exit(1);
}

void mostrar_advertencia(const char* mensaje, int linea) {
    fprintf(stderr, "Advertencia en la linea %d: %s\n", linea, mensaje);
}

int es_tipo_valido(const char* tipo) {
    for (int i = 0; i < sizeof(tipos_validos) / sizeof(tipos_validos[0]); i++) {
        if (strcmp(tipo, tipos_validos[i]) == 0) {
            return true;  
        }
    }
    return false; 
}

void analizar_inicializacion_variables(Nodo* arbol, char* linea, int* num_linea, FILE* archivo, char* ultima_linea) {
    char buffer[256];
    trim((char*)linea);
    Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
    agregar_hijo(arbol, nodo_keyword);
    int count = 0;
    while(fgets(buffer, sizeof(buffer), archivo)) {
        if (starts_with(buffer, "begin") || starts_with(buffer, "procedure") || starts_with(buffer, "function") || starts_with(buffer, "writeln")) {
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
    }
    strcpy(linea, buffer); }

void analizar_palabra_clave(Nodo* arbol, const char* linea, int *num_linea, bool fromF_Or_P, FILE* archivo) {
    char buffer[256]; 
    trim((char*)linea);
    char palabra_temp[256];
    strcpy(palabra_temp, linea);
    trim_semicolon(palabra_temp);

    if (strncmp(palabra_temp, "end", 3) == 0) {
        if (strlen(linea) > 3) {
            char last_char = linea[strlen(linea) - 1];
            if (last_char == '.' || last_char == ';') {
                Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
                agregar_hijo(arbol, nodo_keyword);
                return;
            }
        }
    }
   
    if (es_palabra_clave_similar(palabra_temp, *num_linea)) {
        return;
    }

    const int num_palabras_clave = sizeof(palabras_clave) / sizeof(palabras_clave[0]);
char linea_copia[256];
strcpy(linea_copia, linea);
trim_semicolon(linea_copia); 

bool palabra_clave_valida = false;
for (int i = 0; i < num_palabras_clave; i++) {
    if (strcmp(linea_copia, palabras_clave[i]) == 0) {
        palabra_clave_valida = true;
        break;
    }
}
if (!palabra_clave_valida) {
    mostrar_error("Palabra clave no reconocida", *num_linea, linea);
    return;
}

if (strcmp(linea_copia, "end") == 0 && !(ends_with(linea, ";") || ends_with(linea, "."))) {
    mostrar_error("Error de sintaxis: 'end' debe finalizar con ';' o '.'", *num_linea, linea);
    return;
}


    if ((strcmp(linea, "begin") == 0) && fromF_Or_P) {
        Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
        agregar_hijo(arbol, nodo_keyword);
        return;
    } else if ((strcmp(linea, "begin") == 0) && !fromF_Or_P) {
        Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
        agregar_hijo(arbol, nodo_keyword);

        while (fgets(buffer, sizeof(buffer), archivo)) {
            (*num_linea)++;
            trim(buffer);
            if (buffer[0] == '\0') continue;

            if (starts_with(buffer, "if")) {
                analizar_if(nodo_keyword, buffer, num_linea, archivo);
            }

            if (starts_with(buffer, "while")) {
                analizar_while(nodo_keyword, buffer, num_linea, archivo);
            }

            if (starts_with(buffer, "writeln")) {
                analizar_writeln(nodo_keyword, buffer, *num_linea);
            }

            if (starts_with(buffer, "for")) {
                analizar_for(nodo_keyword, buffer, num_linea, archivo);
            }

            if (starts_with(buffer, "end")) {
                Nodo* nodo_keyword = crear_nodo("palabra_clave", buffer);
                agregar_hijo(arbol, nodo_keyword);
                break;
            }
        }
        return;
    } else {
        Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
        agregar_hijo(arbol, nodo_keyword);
    }
}



void analizar_cabecera_funcion(Nodo* arbol, char* linea, int num_linea, char* nombre_funcion) {
    obtenerNombreFuncion(linea, nombre_funcion, sizeof(nombre_funcion));
    char nombre_funcion_nosirve[256];
    int count = 0;
    bool semicolon = end_with_semicolon(linea);
    if (strncmp(linea, "function ", 9) != 0) {
        mostrar_error("La declaracion debe iniciar con 'function'", num_linea, linea);
    }

    char *abre_paren = strchr(linea, '(');
    char *cierra_paren = strchr(linea, ')');
    if (abre_paren == NULL || cierra_paren == NULL || cierra_paren < abre_paren) {
        mostrar_error("Error en la definicion de los parentesis", num_linea, linea);
    }

    char *dos_puntos = strchr(cierra_paren, ':');
    if (dos_puntos == NULL) {
        mostrar_error("Se esperaba ':' despues de la lista de parametros", num_linea, linea);
    }

    if (!semicolon) {
        mostrar_error("La declaracion debe terminar con ';'", num_linea, linea);
    }

    Nodo* nodo_funcion = crear_nodo("funcion", nombre_funcion); 
    agregar_hijo(arbol, nodo_funcion);
    Nodo *nodo_cabecera_funcion = crear_nodo("cabecera", ""); 
    agregar_hijo(nodo_funcion, nodo_cabecera_funcion);
    char **partes = split_function(linea, &count);
    trim(partes[0]);
     Nodo *nodo_funcion1 = crear_nodo("header pt1", partes[0]); 
    agregar_hijo(nodo_cabecera_funcion, nodo_funcion1);
    Nodo* nodo_dos_puntos = crear_nodo("dos puntos", ":"); 
    agregar_hijo(nodo_cabecera_funcion, nodo_dos_puntos);
    Nodo* nodo_funcion2 = crear_nodo("header pt2", partes[1]); 
    agregar_hijo(nodo_cabecera_funcion, nodo_funcion2);
    if (sscanf(partes[0], "function %[^;];", nombre_funcion_nosirve) == 1) {
        char *contenido_parentesis = extraer_parentesis(partes[0]);
        if (contenido_parentesis != NULL) {
            if (!validar_parametros_funcion(contenido_parentesis, num_linea)) {
                return;
            }
        }
        Nodo* nodo_parentesis1 = crear_nodo("parentesis", "'('");
        Nodo* nodo_parentesis2 = crear_nodo("parametros", contenido_parentesis);
        Nodo* nodo_parentesis3 = crear_nodo("parentesis", "')'");
        agregar_hijo(nodo_funcion1, nodo_parentesis1);
        agregar_hijo(nodo_funcion1, nodo_parentesis2);
        agregar_hijo(nodo_funcion1, nodo_parentesis3);
         if (contenido_parentesis == NULL) {
            free(partes);
            mostrar_error("Funcion mal formada", num_linea, linea);
        }
        char **params = split(contenido_parentesis, ";", &count);
        int elem = contar_elementos(params);
        for (int i = 0; i < elem; i++) {
            Nodo* nodo_parametro = crear_nodo("parametro", params[i]);
            agregar_hijo(nodo_parentesis2, nodo_parametro);
            if (i < elem - 1) {
                Nodo* nodo_coma = crear_nodo("punto y coma", ";");
                agregar_hijo(nodo_parentesis2, nodo_coma);
            }

            char **parametros = split(params[i], ":", &count);
            int count3 = contar_elementos(parametros);
             char **paramsSameType = split(parametros[0], ",", &count);
            int numParamsSameType = contar_elementos(paramsSameType);
            for (int j = 0; j < count3; j++) {
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
                int isValidType = es_tipo_valido(parametros[1]);
                 if (!isValidType) {
                    mostrar_error("Tipo de dato no valido", num_linea, linea);
                }
            }
            trim(partes[1]);
            trim_semicolon(partes[1]);
            toLowerCase(partes[1]);
            int isValidType = es_tipo_valido(partes[1]);
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
    char buffer[256]; 
    bool cabecera_analizada = false; 
    int count = 0;
   if(!cabecera_analizada) {
        analizar_cabecera_funcion(arbol, linea, *num_linea, nombre_funcion);
        cabecera_analizada = true;
    }
   Nodo* nodo_cuerpo_funcion = crear_nodo("cuerpo_funcion", "");
    agregar_hijo(arbol, nodo_cuerpo_funcion);
    

    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        trim(buffer);
         if(buffer[0] == '\0'){
            continue;
        }

          if(strstr(buffer, ":=") != NULL){
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
                mostrar_error("La variable de retorno no coincide con el nombre de la funcion", *num_linea, buffer);
            }
             analizar_asignacion(nodo_cuerpo_funcion, buffer, *num_linea);

             for(int i = 0; i < count; i++) {
                free(partesRetornoFuncion[i]);
            }
            free(partesRetornoFuncion);
        }

        if(contiene_elemento(buffer, palabras_clave, sizeof(palabras_clave) / sizeof(palabras_clave[0]))){
            analizar_palabra_clave(nodo_cuerpo_funcion, buffer, num_linea, true, archivo);{
        }
 


        
        if (strcmp(buffer, "end;") == 0) {
            break;
            }
        }
    }
}

void analizar_expresion(Nodo* arbol, char* expr, int num_linea) {
    removeSpaces(expr);
    trim_semicolon(expr);
    int count = strlen(expr);
    int num_operadores = sizeof(operadoresAritmeticos) / sizeof(operadoresAritmeticos[0]);
    
    for (int i = 0; i < count; i++) {
       
        bool es_operador = false;
        for (int j = 0; j < num_operadores; j++) {  
            int op_len = strlen(operadoresAritmeticos[j]);

            if (i + op_len <= count && strncmp(&expr[i], operadoresAritmeticos[j], op_len) == 0) {
                Nodo* nodo_operador = crear_nodo("operador", operadoresAritmeticos[j]);
                agregar_hijo(arbol, nodo_operador);
                es_operador = true;
                i += op_len - 1; 
                break;
            }
        }

        if (!es_operador) {
            char operando[2] = {expr[i], '\0'};
            Nodo* nodo_operando = crear_nodo("operando", operando);
            agregar_hijo(arbol, nodo_operando);
        }
    }
}

void analizar_asignacion(Nodo* arbol, const char* linea, int num_linea){
    printf("ANALIZAR ASIGNACION\n");
    printf("linea: %s\n", linea);
     if (!validar_asignacion(linea, num_linea)) {
        return;
    }
    
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
    for (int i = 0; i < count; i++) {
        free(partes[i]);
    }
    free(partes);
}

void analizar_procedure(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo, char* nombre_procedure) {
    char buffer[256]; 
	trim((char*)linea);
    obtenerNombreProcedure(linea, nombre_procedure, sizeof(nombre_procedure));
    
    if (strncmp(linea, "procedure ", 10) != 0) {
        mostrar_error("La declaracion debe iniciar con 'procedure'", *num_linea, linea);
    }

    if (!end_with_semicolon(linea)) {
        mostrar_error("La declaracion debe terminar con ';'", *num_linea, linea);
    }

    Nodo* nodo_procedure = crear_nodo("procedure", nombre_procedure);
    agregar_hijo(arbol, nodo_procedure);

    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        trim(buffer);
        
        if (buffer[0] == '\0') {
            
            continue;
        }
        if(contiene_palabra_clave(buffer, palabras_clave, sizeof(palabras_clave) / sizeof(palabras_clave[0]))){
            analizar_palabra_clave(nodo_procedure, buffer, num_linea, true, archivo);
        }

        if(strstr(buffer, "writeln") != NULL){
            analizar_writeln(nodo_procedure, buffer, *num_linea);
        }

        if(strcmp(buffer, "end;") == 0){
            break;
        }
    }
}

void analizar_writeln(Nodo* arbol, const char* linea, int num_linea) {
    char contenido[256];
    contenidoWriteln((char*)linea, contenido);
    trim((char*)linea);
    if (!end_with_semicolon(linea)) {
        mostrar_error("La declaraciÃƒÂ³n debe terminar con ';'", num_linea, linea);
    }
    if (strstr(linea, "writel") != NULL && strstr(linea, "writeln") == NULL) {
        mostrar_error("Comando incorrecto. ¿Quiso escribir 'writeln'?", num_linea, linea);
        return;
    }
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
            mostrar_error("Comilla simple faltante", num_linea, linea);
        }
    }

    size_t len = strlen(contenido_en_parentesis);

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
    }
    Nodo* contenido_writeln = crear_nodo("contenido", contenido_en_parentesis);
    agregar_hijo(nodo_writeln, contenido_writeln);
    
}

void analizar_if(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo) {
    char condicion[256];
    char buffer[256];
    extraer_condicion_if(linea, condicion);
    trim((char*)linea);
    
    if (strstr(linea, "then") != NULL && !starts_with(linea, "if")) {
        mostrar_error("'then' debe ser precedido por 'if'", *num_linea, linea);
        return;
    }

    int terminaConThen = ends_with(linea, "then");
    if (!terminaConThen) {
        mostrar_error("La estructura if debe terminar con 'then'", *num_linea, linea);
    }

    if (!validar_condicion(condicion, *num_linea)) {
        return;
    }
    Nodo* nodo_if_statement = crear_nodo("if_statement", linea);
    agregar_hijo(arbol, nodo_if_statement);
    Nodo* nodo_if = crear_nodo("if", "if");
    agregar_hijo(nodo_if_statement, nodo_if);
    Nodo* contenido_if = crear_nodo("contenido", condicion);
    agregar_hijo(nodo_if, contenido_if);
    int num_operadores = sizeof(operadoresDeComparacion) / sizeof(operadoresDeComparacion[0]);
    int count = 0;
    for (int i = 0; i < sizeof(operadoresDeComparacion) / sizeof(operadoresDeComparacion[0]); i++) {
        if (strstr(condicion, operadoresDeComparacion[i]) != NULL) {
            char** partesCondicion = split(condicion, operadoresDeComparacion[i], &count);
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
    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        trim((char*)buffer);
        if (linea[0] == '\0') {
            continue;
        }
        if (starts_with(buffer, "while") || starts_with(buffer, "for")) {
            strcpy((char*)linea, buffer);
            break;
        }
        Nodo* nodo_sentencia = crear_nodo("sentencia", "");
        agregar_hijo(nodo_if_statement, nodo_sentencia);
        if (strstr(buffer, "writeln") != NULL) {
            analizar_writeln(nodo_sentencia, buffer, *num_linea);
        }
    }
}

void analizar_while(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo){
    char condicion[256];
    char buffer[256];
    trim((char*)linea);
    extraer_condicion_while(linea, condicion);
    if (!validar_condicion(condicion, *num_linea)) {
        return;
    }
    int terminaConDo = ends_with(linea, "do");
    printf("Termina con do: %i\n", terminaConDo);
    if(!terminaConDo){
        mostrar_error("La estructura while debe terminar con 'do'", *num_linea, linea);
    }
    Nodo* while_statement = crear_nodo("while_statement", linea);
    agregar_hijo(arbol, while_statement);
    Nodo* nodo_while = crear_nodo("while", "while");
    agregar_hijo(while_statement, nodo_while);
    trim(condicion);
    Nodo* contenido_while = crear_nodo("contenido", condicion);
    agregar_hijo(nodo_while, contenido_while);
    int num_operadores = sizeof(operadoresDeComparacion) / sizeof(operadoresDeComparacion[0]);
    int count = 0;
    for(int i = 0; i < num_operadores; i++){
        if(strstr(condicion, operadoresDeComparacion[i]) != NULL){
           char** partesCondicion = split(condicion, operadoresDeComparacion[i], &count);
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
        if(starts_with(buffer, "if") || starts_with(buffer, "for") || starts_with(buffer, "while")){
            strcpy((char*)linea, buffer);
            break;
        }
        Nodo* nodo_sentencia = crear_nodo("sentencia", "");
        agregar_hijo(while_statement, nodo_sentencia);
        if(strstr(buffer, "writeln") != NULL){
            analizar_writeln(nodo_sentencia, buffer, *num_linea);
        }
    }

}

void analizar_for(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo){
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
    if (!strstr(linea, "to") && !strstr(linea, "downto")) {
        mostrar_error("El ciclo for debe incluir 'to' o 'downto'", *num_linea, linea);
        return;
    }
    if (!validar_asignacion(inicializacion, *num_linea)) {
        return;
    }
    extraer_condicion_for(linea, inicializacion, operador_control, final);
    printf("Incializacion del for: %s\n", inicializacion);
    char** partesInicializacion = split(inicializacion, ":=", &count);
    Nodo* for_statement = crear_nodo("for_statement", linea);
    agregar_hijo(arbol, for_statement);
    Nodo* nodo_for = crear_nodo("for", "for");
    agregar_hijo(for_statement, nodo_for);
    if(inicializacion[0]=='\0'){
        mostrar_error("Operador de control to o downto no encontrado", *num_linea, linea);
    }
    analizar_asignacion(for_statement, inicializacion, *num_linea);
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
           continue;
        }
       if(starts_with(buffer, "if") || starts_with(buffer, "for") || starts_with(buffer, "while")){
            strcpy((char*)linea, buffer);
            break;
        }

        if(starts_with(buffer, "end")){
            analizar_palabra_clave(arbol, buffer, num_linea, false, archivo);
            break;
        }
        Nodo* nodo_sentencia = crear_nodo("sentencia", "");
        agregar_hijo(for_statement, nodo_sentencia);
        if(strstr(buffer, "writeln") != NULL){
            analizar_writeln(nodo_sentencia, buffer, *num_linea);
        }
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
        return 1; 
    }

    const char *text = "Sumar := a + b + c * d;";

    Nodo* arbol = crear_nodo("programaPrueba", "");
    char linea[256];
    char nombre_funcion[50];
    char nombre_procedure[50];  
    int num_linea = 0;
    while (fgets(linea, sizeof(linea), archivo)) {
        trim(linea);
        char ultima_linea[256];
        if (*linea == '\0') {
            num_linea++;
            continue;
        };  
        for (int i = 0; linea[i]; i++) linea[i] = tolower(linea[i]);

        char palabra_temp[256];
        strcpy(palabra_temp, linea);
        trim_semicolon(palabra_temp);
        if (es_palabra_clave_similar(palabra_temp, num_linea)) {
            continue;
        }

        if(starts_with(linea, "var")) {

            analizar_inicializacion_variables(arbol, linea, &num_linea, archivo, ultima_linea);

        }
        if (starts_with(linea, "function")) {
            analizar_funcion(arbol, linea, &num_linea, archivo, nombre_funcion);
        }else if(starts_with(linea, "procedure")){
            analizar_procedure(arbol, linea, &num_linea, archivo, nombre_procedure);
        }else if(starts_with(linea, "if")){
            char condicion[256];
            extraer_condicion_if(linea, condicion);
        }else if(starts_with(linea, "begin")){
            analizar_palabra_clave(arbol, linea, &num_linea, false, archivo);
        }else if(starts_with(linea, "while")){
            analizar_while(arbol, linea, &num_linea, archivo);
        }else if(starts_with(linea, "for")){
        }else if(starts_with(linea, "writeln")){
            analizar_writeln(arbol, linea, num_linea);
        }
        num_linea++;
    }

    fclose(archivo);
    imprimir_arbol(arbol, 0);

    return 0; 
}