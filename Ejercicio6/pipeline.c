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

void
addArgs(char *argv[], Command *Commands[]) {

    for (int i = 0; i < NUM_PARAMS; i++ ) {
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
    
    for (int numCommand = 0; numCommand<NUM_PARAMS; numCommand++) {
        for (int i = 0; i < (Commands[numCommand]->numArgumentos); i++) {
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


    int child1, child2, child3;

    switch (child1 = fork()) { // ---------- HIJO 1
        case -1:
            err(EXIT_FAILURE, "Theres an error with the child proccess");
        case 0:
            close(pipes[0][0]);
            dup2(pipes[0][1], STDOUT_FILENO);
            close(pipes[0][1]);
            execlp("/bin/ls", "ls", "-l", "/", NULL);
        default:
            close (pipes[0][1]);
            switch (child2 = fork()) { // ---------- HIJO 2
                case -1:
                    err(EXIT_FAILURE, "Theres an error with the child proccess");
                case 0:
                    
                    dup2(pipes[0][0], STDIN_FILENO);
                    close(pipes[0][0]);

                    close(pipes[1][0]);
                    dup2(pipes[1][1], STDOUT_FILENO);
                    close(pipes[1][1]);
                    execlp("/bin/grep","grep", "u",NULL);
                default:
                    
                    close(pipes[1][1]);
                    close(pipes[0][0]);
                    switch (child3 = fork()) { // ---------- HIJO 3
                        case -1:
                            err(EXIT_FAILURE, "Theres an error with the child proccess");
                        case 0:
                            dup2(pipes[1][0], STDIN_FILENO);
                            close(pipes[1][0]);
                            execlp("/usr/bin/wc","wc", "-l",NULL);
                        default:
                            close(pipes[1][0]);

                            int status;
                            wait(&status);
                            wait(&status);
                            wait(&status);

                    }

            }

    }





    //close(pipefd1[0]); //CERRAMOS EL EXTREMO DE LECTURA PORQUE VAMOS A ESCRIBIR
    //execCmd(Commands[0], STDIN_FILENO ,pipefd1[1]);


    //close(pipefd1[1]); //CERRAMOS EL EXTREMO DE ESCRITURA PORQUE VAMOS A LEER
    //close(pipefd2[0]);
    //execCmd(Commands[1], pipefd1[0] ,pipefd2[1]);
    //execCmd(Commands[2], pipefd2[0], STDOUT_FILENO);

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