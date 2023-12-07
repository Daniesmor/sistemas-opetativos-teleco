#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>


enum {
    INITIAL_MAX_ARGS = 10,
};


struct Command {
    char nombre[50]; // Campo para el nombre del comando
    int numArgumentos; // Contador para llevar el seguimiento de la cantidad de argumentos
    char **argumentos;
};

typedef struct Command Command;


// ------------------------------ ACCIONES BASICAS ------------------------------------------------------------------

void memLocateFailed() {
    err(EXIT_FAILURE, "Error: Could not allocate memory for the argument.\n");
}


// --------------------------------- FIN ACCIONES BASICAS  --------------------------------------------------------

// -------------------------------- CHECKEOS DE SEGURIDAD -------------------------------------------------------------------------------






// ------------------------------ FIN DE CHECKEOS DE SEGURIDAD ------------------------------------------------------------------------

void
initializerCommands(char *argv[], Command *Commands[]) {

    for (int i = 0; i<3; i++) {
        Commands[i] = malloc(sizeof(Command));
        
        Commands[i]->argumentos = NULL;
        Commands[i]->numArgumentos = 0;
        //printf("%s \n", Commands[i]->nombre);
    }


}

void
addArgs(char *argv[], Command *Commands[]) {

    for (int i = 0; i < 3; i++ ) {
        char *token;
        char *saveptr;

        char *argv_copy = strdup(argv[i+1]); // PARA TRABAJAR CON STRTOK_ES CONVENIENTE USAR UNA COPIA DE ARGV

        token = strtok_r(argv_copy, " ", &saveptr);
        strcpy(Commands[i]->nombre, token); // ASIGNAMOS EL NOMBRE DE COMANDO


        while (token != NULL) {
            token = strtok_r(NULL, " ", &saveptr);

            if (token != NULL) {
                Commands[i]->argumentos = realloc(Commands[i]->argumentos, (Commands[i]->numArgumentos + 1)* sizeof(char *));
                if (Commands[i]->argumentos == NULL) {
                    memLocateFailed();
                }


                Commands[i]->argumentos[Commands[i]->numArgumentos] = malloc((strlen(token) + 1) * sizeof(char));
                if (Commands[i]->argumentos[Commands[i]->numArgumentos] == NULL) {
                    memLocateFailed();
                }

                
                strcpy(Commands[i]->argumentos[Commands[i]->numArgumentos], token);
                Commands[i]->numArgumentos++;
                //token = strtok_r(NULL, " ", &saveptr);
            }
            
        }

        free(argv_copy);
    }

    

    
    
}

void
freeMem(Command *Commands[]) {
    
    for (int numCommand = 0; numCommand<3; numCommand++) {
        for (int i = 0; i < (Commands[numCommand]->numArgumentos); i++) {
            free(Commands[numCommand]->argumentos[i]);
        }
        free(Commands[numCommand]->argumentos);
        free(Commands[numCommand]);
    }

    

}

void
printCommand(Command *Commands[]) {
    for (int numCommand = 0; numCommand <3; numCommand++) {
        printf("%s \n", Commands[numCommand]->nombre);
        for (int i = 0; i < Commands[numCommand]->numArgumentos; i++) {
            printf("%s \n", Commands[numCommand]->argumentos[i]);
        }
    }
    
}

int 
main(int argc, char *argv[]) {

    if (argc < 3) {
        exit(EXIT_FAILURE);
        err(EXIT_FAILURE,"ERROR: Yoy must enther 3 commands. \n");
    }

    //Commands Commands_with_args;
    Command *Commands[3];
    initializerCommands(argv, Commands);
    
    addArgs(argv,Commands);
    printCommand(Commands);
    freeMem(Commands);
    //separate_commands(argv, &Command1);

    

    return 0;
}