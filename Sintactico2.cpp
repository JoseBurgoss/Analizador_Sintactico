#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

const char* palabras_clave[] = {"begin", "end", "if", "then", "else", "while", "do", "for", "to", "downto", "repeat", 
								"until", "case", "of", "function", "procedure", "const", "type", "record", "array", 
                                "var"};

const char* tipos_validos[] = {"integer", "string", "real", "boolean", "char"};

const char* operadores[] = {"*", "/","mod","+","-"};


typedef struct Nodo {
    char tipo[20];      
    char valor[50];      
    struct Nodo** hijos;  
    int num_hijos;       
} Nodo;

Nodo* crear_nodo(const char* tipo, const char* valor) {
    // printf("CREAR NODO\n");
    // printf("TIPO: %s\n", tipo);
    // printf("VALOR: %s\n", valor);
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

bool end_with_semicolon(const char* str) {
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
    printf("TRIM\n");
    printf("str: %s\n", str);
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
    // printf("inicio: %s\n", inicio);
    // printf("fin: %s\n", fin);
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
    //printf("index: %i\n", index);
    *count = index; // Guardamos el número de elementos en `count`

    free(copia);  // Liberamos la copia original

    return resultado; // Retornamos el array de punteros a strings
}

void mostrar_error(const char* mensaje, int linea, const char* detalle) {
    fprintf(stderr, "Error en la linea %d: %s -> %s\n", linea, mensaje, detalle);
    exit(1);
}

void mostrar_advertencia(const char* mensaje, int linea) {
    fprintf(stderr, "Advertencia en la linea %d: %s\n", linea, mensaje);
}

int es_tipo_valido(const char* tipo) {
    printf("TIPO: %s\n", tipo);
    for (int i = 0; i < sizeof(tipos_validos) / sizeof(tipos_validos[0]); i++) {
        printf("Estoy evaluando %s con %s (<- Este fue el que recibi por parametro)\n", tipos_validos[i], tipo);
        if (strcmp(tipo, tipos_validos[i]) == 0) {
            return true;  
        }
    }
    return false; 
}

void analizar_funcion(Nodo* arbol, const char* linea, int num_linea) {
    printf("ANALIZAR FUNCION\n");
    char nombre_funcion[50];
    int count = 0;
    printf("linea %i: %s\n", num_linea, linea);
    bool semicolon = end_with_semicolon(linea);

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

    Nodo* nodo_funcion = crear_nodo("funcion", ""); // Create a node for the function
    agregar_hijo(arbol, nodo_funcion);
    Nodo *nodo_cabecera_funcion = crear_nodo("cabecera", ""); // Create a node for the function header
    agregar_hijo(nodo_funcion, nodo_cabecera_funcion);
    char **partes = split_function(linea, &count);
    trim(partes[0]);
    printf("Header pt1 %s\n", partes[0]);
    Nodo *nodo_funcion1 = crear_nodo("header pt1", partes[0]); // Create a node for the first part of the header
    agregar_hijo(nodo_cabecera_funcion, nodo_funcion1);
    Nodo* nodo_dos_puntos = crear_nodo("dos puntos", ":"); // Create a node for the colon
    agregar_hijo(nodo_cabecera_funcion, nodo_dos_puntos);
    Nodo* nodo_funcion2 = crear_nodo("header pt2", partes[1]); // Create a node for the second part of the header
    agregar_hijo(nodo_cabecera_funcion, nodo_funcion2);
    printf("Primera parte de la cadena de la funcion: %s\n", partes[0]);
    printf("Segunda parte de la cadena de la funcion: %s\n", partes[1]);
    if (sscanf(partes[0], "function %[^;];", nombre_funcion) == 1) {
        char *contenido_parentesis = extraer_parentesis(partes[0]);
        Nodo* nodo_parentesis1 = crear_nodo("parentesis", "'('");
        Nodo* nodo_parentesis2 = crear_nodo("parametros", contenido_parentesis);
        Nodo* nodo_parentesis3 = crear_nodo("parentesis", "')'");
        agregar_hijo(nodo_funcion1, nodo_parentesis1);
        agregar_hijo(nodo_funcion1, nodo_parentesis2);
        agregar_hijo(nodo_funcion1, nodo_parentesis3);
        printf("Contenido entre paréntesis: %s\n", contenido_parentesis);
        if (contenido_parentesis == NULL) {
            mostrar_error("Funcion mal formada", num_linea, linea);
        }
        char **params = split(contenido_parentesis, ";", &count);
        int elem = contar_elementos(params);
        for (int i = 0; i < elem; i++) {
            Nodo* nodo_parametro = crear_nodo("parametro", params[i]);
            agregar_hijo(nodo_parentesis2, nodo_parametro);
            printf("Param %i: %s\n", i, params[i]);
            if (i < elem - 1) {
                printf("Agregra punto y coma ya que i es %i y elem es %i\n", i, elem);
                Nodo* nodo_coma = crear_nodo("punto y coma", ";");
                agregar_hijo(nodo_parentesis2, nodo_coma);
            }

            char **parametros = split(params[i], ":", &count);
            int count3 = contar_elementos(parametros);
            printf("Numero de elementos count3: %i\n", count3);
            char **paramsSameType = split(parametros[0], ",", &count);
            int numParamsSameType = contar_elementos(paramsSameType);
            printf("Numero de parametros del mismo tipo: %i\n", numParamsSameType);
            for (int j = 0; j < count3; j++) {
                printf("Parametro j %i: %s\n", j, parametros[j]);
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
                printf("%s tiene %i letras\n", parametros[1], chars);

                int isValidType = es_tipo_valido(parametros[1]);
                printf("isValidType: %i\n", isValidType);
                if (!isValidType) {
                    mostrar_error("Tipo de dato no valido", num_linea, linea);
                }
            }
            trim(partes[1]);
            trim_semicolon(partes[1]);
            toLowerCase(partes[1]);
            int isValidType = es_tipo_valido(partes[1]);
            printf("isValidReturnType: %i\n", isValidType);
            if (!isValidType) {
                mostrar_error("Tipo de retorno de la funcion dato no valido", num_linea, linea);
            }
            free(parametros);
        }
        free(partes);
        free(contenido_parentesis);
        free(params);
    } else {
        //mostrar_error("Declaracion 'program' mal formada", num_linea, linea);
    }
}


void analizar_expresion(Nodo* arbol, char* expr, int num_linea) {
    printf("ANALIZAR EXPRESION\n");
    removeSpaces(expr);
    trim_semicolon(expr);   
    printf("EXPRESION: %s\n", expr);
    int count = strlen(expr);
    printf("COUNT: %i\n", count);

    for (int i = 0; i < count; i++) {
        printf("CHAR: %c\n", expr[i]);
        bool es_operador = false;

        // Verificar si el carácter es un operador
        for (int j = 0; j < sizeof(operadores) / sizeof(operadores[0]); j++) {
            if (strncmp(&expr[i], operadores[j], strlen(operadores[j])) == 0) {
                printf("OPERADOR: %s\n", operadores[j]);
                Nodo* nodo_operador = crear_nodo("operador", operadores[j]);
                agregar_hijo(arbol, nodo_operador);
                es_operador = true;
                i += strlen(operadores[j]) - 1; // Avanzar el índice para saltar el operador completo
                break;
            }
        }

        if (!es_operador) {
            char operando[2] = {expr[i], '\0'};
            printf("OPERANDO: %s\n", operando);
            Nodo* nodo_operando = crear_nodo("operando", operando);
            agregar_hijo(arbol, nodo_operando);
        }
    }
}

void analizar_asignacion(Nodo* arbol, const char* linea, int num_linea){
    printf("ANALIZAR ASIGNACION\n");
    char variable[50], expresion[100];
    int count = 0;
    char **partes = split(linea, ":=", &count);
    if (count != 2) {
        mostrar_error("Asignacion mal formada", num_linea, linea);
    }
    trim(partes[0]);
    trim(partes[1]);
    toLowerCase(partes[0]);
    toLowerCase(partes[1]);
    printf("Parte 1: %s\n", partes[0]);
    printf("Parte 2: %s\n", partes[1]);
    Nodo* nodo_asignacion = crear_nodo("asignacion", "");
    agregar_hijo(arbol, nodo_asignacion);
    Nodo* nodo_variable = crear_nodo("variable", partes[0]);
    Nodo* nodo_asignacion_operador = crear_nodo("asignacion_operador", ":=");
    Nodo* nodo_expresion = crear_nodo("expresion", partes[1]);
    agregar_hijo(nodo_asignacion, nodo_variable);
    agregar_hijo(nodo_asignacion, nodo_asignacion_operador);
    agregar_hijo(nodo_asignacion, nodo_expresion);
    analizar_expresion(nodo_expresion, partes[1], num_linea);
    free(partes);
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

    // int count = 0;
    // int count2 = 0;
    //const char *text = "function Sumar(a, b: intEger; c: STRING; d: boolean): integer;";
    const char *text = "Sumar := a + b + c * d;";
    // char** partes = split_function(text, &count);
    // printf("%s\n", partes[0]);
    // char *contenido_parentesis = extraer_parentesis(partes[0]);
    // char** partes2 = split(contenido_parentesis, ";", &count2);
    // for (int i = 0; i < count2; i++) {
    //     printf("Parte %i: %s\n", i, partes2[i]);
    //     free(partes2[i]);
    // }
    // printf("Contenido entre paréntesis: %s\n", contenido_parentesis);
    // if(contenido_parentesis != NULL) {
    //     printf("Contenido entre paréntesis: %s\n", contenido_parentesis);
    //     free(contenido_parentesis);
    // }
    // for (int i = 0; i < count; i++) {
    //     printf("Parte %i: %s\n", i, partes[i]);
    //     free(partes[i]);
    // }

    

    Nodo* arbol = crear_nodo("lineaPrueba", "");
    analizar_asignacion(arbol, text, 1);
    //analizar_funcion(arbol, text, 1);
    char linea[256];
    int num_linea = 0;
    while (fgets(linea, sizeof(linea), archivo)) {
        num_linea++;
        //printf("a");
        //printf("%i  %s", num_linea, linea);
        //linea[strcspn(linea, "\n")] = 0;  
        //analizar_linea(arbol, linea, num_linea);
        //analizar_funcion(arbol, linea, num_linea);
    }
     
    fclose(archivo);
   	//holaMundo();
    imprimir_arbol(arbol, 0);

    return 0;
}