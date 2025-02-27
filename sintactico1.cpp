#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Nodo {
    char tipo[20];      
    char valor[50];      
    struct Nodo** hijos;  
    int num_hijos;       
} Nodo;

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

void mostrar_error(const char* mensaje, int linea, const char* detalle) {
    fprintf(stderr, "Error en la linea %d: %s -> %s\n", linea, mensaje, detalle);
    exit(1);
}

void mostrar_advertencia(const char* mensaje, int linea) {
    fprintf(stderr, "Advertencia en la linea %d: %s\n", linea, mensaje);
}

int es_tipo_valido(const char* tipo) {
    
    for (int i = 0; i < sizeof(tipos_validos) / sizeof(tipos_validos[0]); i++) {
        if (strcmp(tipo, tipos_validos[i]) == 0) {
            return 1;  
        }
    }
    return 0; 
}

void analizar_writeln(Nodo* arbol, const char* linea, int num_linea) {
    char argumento[100];
    if (sscanf(linea, "writeln('%[^']');", argumento) == 1 || sscanf(linea, "writeln(%[^)]);", argumento) == 1) {
        Nodo* nodo_writeln = crear_nodo("writeln", "");
        Nodo* nodo_argumento = crear_nodo("argumento", argumento);
        agregar_hijo(nodo_writeln, nodo_argumento);
        agregar_hijo(arbol, nodo_writeln);
    } else {
        mostrar_error("Sentencia 'writeln' mal formada", num_linea, linea);
    }
}

void analizar_declaracion(Nodo* arbol, const char* linea, int num_linea) {
    char nombre_var[50], tipo_var[50];
    if (sscanf(linea, "%[^:]: %[^;];", nombre_var, tipo_var) == 2) {
        if (es_tipo_valido(tipo_var)) {
            Nodo* nodo_declaracion = crear_nodo("declaracion", nombre_var);
            Nodo* nodo_tipo = crear_nodo("tipo", tipo_var);
            agregar_hijo(nodo_declaracion, nodo_tipo);
            agregar_hijo(arbol, nodo_declaracion);
        } else {
            mostrar_error("Tipo de dato no valido", num_linea, tipo_var);
        }
    } else {
        mostrar_error("Declaracion de variable mal formada", num_linea, linea);
    }
}

void analizar_asignacion(Nodo* arbol, const char* linea, int num_linea) {
    char variable[50], expresion[100];
    if (sscanf(linea, "%[^ ] := %[^;];", variable, expresion) == 2) {
        Nodo* nodo_asignacion = crear_nodo("asignacion", variable);
        Nodo* nodo_expresion = crear_nodo("expresion", expresion);
        agregar_hijo(nodo_asignacion, nodo_expresion);
        agregar_hijo(arbol, nodo_asignacion);
    } else {
        mostrar_error("Asignacion mal formada", num_linea, linea);
    }
}

void analizar_for(Nodo* arbol, const char* linea, int num_linea) {
    char variable[50], inicio[50], fin[50];
    if (sscanf(linea, "for %[^ ] := %[^ ] to %[^ ] do", variable, inicio, fin) == 3) {
        Nodo* nodo_for = crear_nodo("for", "");
        Nodo* nodo_variable = crear_nodo("variable", variable);
        Nodo* nodo_inicio = crear_nodo("inicio", inicio);
        Nodo* nodo_fin = crear_nodo("fin", fin);
        agregar_hijo(nodo_for, nodo_variable);
        agregar_hijo(nodo_for, nodo_inicio);
        agregar_hijo(nodo_for, nodo_fin);
        agregar_hijo(arbol, nodo_for);
    } else {
        mostrar_error("Estructura 'for' mal formada", num_linea, linea);
    }
}

void analizar_palabra_clave(Nodo* arbol, const char* linea, int num_linea) {
    const char* palabras_clave[] = {"begin", "end", "if", "then", "else", "while", "do", "for", "to", "downto", "repeat", "until", "case", "of", "function", "procedure", "const", "type", "record", "array", "var"};
    for (int i = 0; i < sizeof(palabras_clave) / sizeof(palabras_clave[0]); i++) {
        if (strstr(linea, palabras_clave[i]) == linea) {
            Nodo* nodo = crear_nodo("palabra_clave", palabras_clave[i]);
            agregar_hijo(arbol, nodo);
            return;
        }
    }
    analizar_declaracion(arbol, linea, num_linea);
}

void analizar_program(Nodo* arbol, const char* linea, int num_linea) {
    char nombre_programa[50];
    if (sscanf(linea, "program %[^;];", nombre_programa) == 1) {
        Nodo* nodo_program = crear_nodo("program", nombre_programa);
        agregar_hijo(arbol, nodo_program);
    } else {
        mostrar_error("Declaracion 'program' mal formada", num_linea, linea);
    }
}

void analizar_linea(Nodo* arbol, char* linea, int num_linea) {
    while (*linea == ' ' || *linea == '\t') linea++;
    if (*linea == '\0') return;  // Ignorar lineas vacias
    for (int i = 0; linea[i]; i++) linea[i] = tolower(linea[i]);
    
    if (strstr(linea, "program") == linea) {
        analizar_program(arbol, linea, num_linea);
    } else if (strstr(linea, "writeln") == linea) {
        analizar_writeln(arbol, linea, num_linea);
    } else if (strstr(linea, "for") == linea) {
        analizar_for(arbol, linea, num_linea);
    } else if (strstr(linea, ":=") != NULL) {
        analizar_asignacion(arbol, linea, num_linea);
    } else {
        analizar_palabra_clave(arbol, linea, num_linea);
    }
}

void imprimir_arbol(Nodo* nodo, int nivel) {
    for (int i = 0; i < nivel; i++) //printf("  ");  
    //printf("%s(%s)\n", nodo->tipo, nodo->valor);  

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

    Nodo* arbol = crear_nodo("programa", "");
    char linea[256];
    int num_linea = 0;
    while (fgets(linea, sizeof(linea), archivo)) {
        num_linea++;
        linea[strcspn(linea, "\n")] = 0;  
        analizar_linea(arbol, linea, num_linea);
    }
    fclose(archivo);

    imprimir_arbol(arbol, 0);

    return 0;
}