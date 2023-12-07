#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    INITIAL_MAX_ARGS = 10,
};


struct Commands {
    char first_command[50];
    char seccond_command[50];
    char third_command[50];
};

typedef struct Commands Commands;

struct Command {
    char nombre[50]; // Campo para el nombre del comando
    int numArgumentos; // Contador para llevar el seguimiento de la cantidad de argumentos
    char **argumentos;
};

typedef struct Command Command;

void
initializerCommands(char *argv[], Command *Comando1) {

    strcpy(Comando1->nombre, argv[1]);
    Comando1->argumentos = NULL;
    Comando1->numArgumentos = 0;

}

void
addArgs(char *argv[], Command *Comando1) {
    
    Comando1->argumentos = realloc(Comando1->argumentos, (Comando1->numArgumentos + 1)* sizeof(char *));
    if (Comando1->argumentos == NULL) {
        printf("Error: no se pudo asignar memoria para el argumento.\n");
        exit(EXIT_FAILURE);
    }

    Comando1->argumentos[Comando1->numArgumentos] = malloc((strlen("argumento") + 1) * sizeof(char));
    if (Comando1->argumentos[Comando1->numArgumentos] == NULL) {
        printf("Error: no se pudo asignar memoria para el argumento.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(Comando1->argumentos[Comando1->numArgumentos], "argumento");
    Comando1->numArgumentos++;
}

void
freeMem(Command *Comando1) {
    
    for (int i = 0; i < Comando1->numArgumentos; i++) {
        free(Comando1->argumentos[i]);
    }
    free(Comando1->argumentos);

}

void
printCommand(Command *Comando1) {
    printf("%s \n", Comando1->nombre);
    for (int i = 0; i < Comando1->numArgumentos; i++) {
        printf("%s \n", Comando1->argumentos[i]);
    }
}

int 
main(int argc, char *argv[]) {

    //Commands Commands_with_args;
    Command Comando1;
    initializerCommands(argv, &Comando1);
    addArgs(argv, &Comando1);
    printCommand(&Comando1);
    freeMem(&Comando1);
    //separate_commands(argv, &Comando1);

    

    return 0;
}