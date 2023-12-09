#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATH1 "/bin/"
#define PATH2 "/usr/bin/"

enum {
    INITIAL_MAX_ARGS = 10,
    COMMAND_LENGTH = 256,
    MAX_PATH = 1024,
    NUM_PARAMS = 3,
};


struct Command {
    char nombre[COMMAND_LENGTH]; // Campo para el nombre del comando
    char path[MAX_PATH];
    int numArgumentos; // Contador para llevar el seguimiento de la cantidad de argumentos
    char **argumentos;
};

typedef struct Command Command;


// ------------------------------ ACCIONES BASICAS ------------------------------------------------------------------

void memLocateFailed() {
    err(EXIT_FAILURE, "Error: Could not allocate memory for the argument.\n");
}

char** reserve_args(Command *cmd) {

    cmd->argumentos = realloc(cmd->argumentos, (cmd->numArgumentos + 1) * sizeof(char *));
    return cmd->argumentos;

}

char ** reserve_args_token(Command *cmd, int token_size) {
    cmd->argumentos[cmd->numArgumentos] = malloc((token_size + 1) * sizeof(char));
    return cmd->argumentos;
}

void
assignCommandName(Command *cmd, char *name) {
    strcpy(cmd->nombre, name); //Para poder asignar un string a un struct, es necesario usar strcpy
}

void 
addArgumentsToken(Command *cmd, char *token) {
    strcpy(cmd->argumentos[cmd->numArgumentos], token);
    cmd->numArgumentos++;
}

void
setLastArgumentNull(Command *cmd) {

    if (reserve_args(cmd) == NULL) {
        // Manejar el error de asignación de memoria
        memLocateFailed();
        //free(argv_copy);
        return;
    }

    cmd->argumentos[cmd->numArgumentos] = NULL; // Establecer el último elemento como NULL
}



// --------------------------------- FIN ACCIONES BASICAS  --------------------------------------------------------

// -------------------------------- CHECKEOS DE SEGURIDAD -------------------------------------------------------------------------------






// ------------------------------ FIN DE CHECKEOS DE SEGURIDAD ------------------------------------------------------------------------

void
initializerCommands(char *argv[], Command *Commands[]) {

    for (int i = 0; i<NUM_PARAMS; i++) {
        Commands[i] = malloc(sizeof(Command));
        
        Commands[i]->argumentos = NULL;
        Commands[i]->numArgumentos = 0;
        //printf("%s \n", Commands[i]->nombre);
    }


}

void addArgs(char *argv[], Command *Commands[]) {
    for (int i = 0; i < NUM_PARAMS; i++ ) {
        char *token;
        char *saveptr;

        char *argv_copy = strdup(argv[i+1]);
        if (argv_copy == NULL) {
            // Manejar el error de asignación de memoria
            memLocateFailed();
            return;
        }

        token = strtok_r(argv_copy, " ", &saveptr);
        assignCommandName(Commands[i], token);
        

        while (token != NULL) {
            if (Commands[i]->numArgumentos !=0) {
                token = strtok_r(NULL, " ", &saveptr);
            }


            if (token != NULL) {
                
                if (reserve_args(Commands[i]) == NULL) {
                    // Manejar el error de asignación de memoria
                    memLocateFailed();
                    free(argv_copy);
                    return;
                }


                if (reserve_args_token(Commands[i], strlen(token)) == NULL) {
                    // Manejar el error de asignación de memoria
                    memLocateFailed();
                    free(argv_copy);
                    return;
                }

                addArgumentsToken(Commands[i], token);
                
            }
        }
        free(argv_copy);
        setLastArgumentNull(Commands[i]);

    }
}

void freeMem(Command *Commands[]) {
    for (int numCommand = 0; numCommand < NUM_PARAMS; numCommand++) {
        for (int i = 0; i < Commands[numCommand]->numArgumentos; i++) {
            free(Commands[numCommand]->argumentos[i]);
        }
        free(Commands[numCommand]->argumentos);
        free(Commands[numCommand]);
    }
}


void
printCommand(Command *Commands[]) {
    for (int numCommand = 0; numCommand <NUM_PARAMS; numCommand++) {
        printf("%s \n", Commands[numCommand]->nombre);
        for (int i = 0; i < Commands[numCommand]->numArgumentos; i++) {
            printf("%s \n", Commands[numCommand]->argumentos[i]);
        }
    }
    
}

int
searchExec(Command *cmd) {
    
    //char path[10 +COMMAND_LENGTH];

    char full_path[10 + COMMAND_LENGTH];

    strcpy(full_path, PATH1);
    strcat(full_path, cmd->nombre);
    if (access(full_path, F_OK) == 0) {
        strcpy(cmd->path, full_path);
        return 0;
    }
    else {
        strcpy(full_path, PATH2);
        strcat(full_path, cmd->nombre);
        if (access(full_path, F_OK)==0) {
            strcpy(cmd->path, full_path);
            return 0;
        }
        else {
            return 1;
        }
    }

    return 0;

}

void
execCmd(Command *cmd) {
    int child;

    switch (child = fork()) {
    case -1:
        err(EXIT_FAILURE, "Theres an error with the child");
    case 0:
        execv(cmd->path, cmd->argumentos);
        exit(0);
    default:    

    }
}

void 
executeCommands(Command *Commands[]) {
    int pipes[NUM_PARAMS-1][2];
    

    for (int i = 0; i < NUM_PARAMS -1; i++) {
        if (pipe(pipes[i]) == -1) {
            err(EXIT_FAILURE, "Theres an error creating the first pipe.");
        }  
    }

    // Creamos un proceso que cambia de ruta hacia el ejecutable para ver si existe (si devuelve 0 ha sido exitoso).
    for (int numCommand = 0; numCommand < NUM_PARAMS; numCommand++) {


        if (searchExec(Commands[numCommand]) != 0) {
            printf("The command %s doesn't exists. \n", Commands[numCommand]->nombre);
            exit(EXIT_FAILURE);
            //printf("Se puede ejecutar el execv \n");
            //printf("%s \n", Commands[numCommand]->path);
            //printf("%s \n", *Commands[i]->argumentos);
            //for (int i = 0; i < Commands[numCommand]->numArgumentos; i++) {
              //  printf("%s \n", Commands[numCommand]->argumentos[i]);
            //}
            //execv(Commands[numCommand]->path, Commands[numCommand]->argumentos);
            //execCmd(Commands[numCommand]);
            //execl("/bin/ls", "-l", "/", NULL);
        }
        
    }


    int child;

    for (int numCommand=0; numCommand<NUM_PARAMS; numCommand++) {
        switch (child = fork()) { // ---------- HIJO 1
            case -1:
                err(EXIT_FAILURE, "Theres an error with the child proccess");
            case 0:
                if (numCommand > 0) { //SI NO ES EL PRIMER COMANDO, LA ENTRADA LA TIENE QUE LEER DEL COMANDO ANTERIOR
                    dup2(pipes[numCommand-1][0], STDIN_FILENO); //Si no es el pirmer comando, deberá leer la entrada de la salida del anterior.
                    close(pipes[numCommand-1][0]); //Como en dup2 ya se ha hecho el duplicado, podemos cerrar el pipe
                    close(pipes[numCommand-1][1]); //Del pipe que lo precede, solo queremos leer la entrada, por lo tanto cerramos la escritura
        
                }
                if (numCommand < NUM_PARAMS -1 ) { //SI NO ES EL ULTIMO COMANDO, LA SALIDA DEBE SER ENVIADA AL SIGUIENTE COMANDO
                    close(pipes[numCommand][0]); // COMO QUEREMOS ESCRIBIR EN EL PIPE QUE LO UNE CON EL SIG COMANDO, PODEMOS CERRAR EL EXTREMO DE LECTURA
                    dup2(pipes[numCommand][1], STDOUT_FILENO); //REDIRIGIMOS LA SALIDA AL PIPE QUE LO UNE CON EL SIG COMANDO
                    close(pipes[numCommand][1]); // COMO YA HEMOS HECHO EL DUPLICADO CON DUP2, LO PODEMOS BORRAR
                }
                // EJECUTAMOS EL COMANDO Y LO MANEJAMOS EN CASO DE ERROR
                execv(Commands[numCommand]->path, Commands[numCommand]->argumentos);
                printf("There's an error executing the comand %s. \n", Commands[numCommand]->nombre);
                exit(EXIT_FAILURE);
            default:
                // UNA VEZ QUE SE EJECUTE EL COMANDO, EL PADRE CERRARÁ LO QUE YA NO SE VA A VOLVER A USAR
                if (numCommand < NUM_PARAMS -1) {
                    close(pipes[numCommand][1]); // Cierra el extremo de escritura del pipe actual
                }

        }
    }

    // Código del padre que espera a todos los hijos
    for (int i = 0; i < NUM_PARAMS; i++) {
        int status;
        wait(&status);
    }

}
    


int 
main(int argc, char *argv[]) {

    if (argc < NUM_PARAMS) {
        printf("You must enter at least %d commands \n", NUM_PARAMS);
        exit(EXIT_FAILURE);
    }

    //Commands Commands_with_args;
    Command *Commands[NUM_PARAMS];
    initializerCommands(argv, Commands);
    
    addArgs(argv,Commands);
    //printCommand(Commands);
    executeCommands(Commands);
    freeMem(Commands);
    //separate_commands(argv, &Command1);

    

    return 0;
}