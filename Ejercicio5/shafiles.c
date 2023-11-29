#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <sys/wait.h>

// ----------------------- ESTRUCTURAS DE DATOS UTILIZADAS --------------------------------------------------------------------------------------

enum {
    MAX_PATH_LENGTH = 256,
    MAX_FILENAME_LENGTH = 100,
    LINE_BUFFER_SIZE = MAX_PATH_LENGTH * sizeof(char),
    FILENAME_BUFFER_SIZE = MAX_FILENAME_LENGTH * sizeof(char),
    READ_PERMISSION = 0444,
    RW_PERMISSION = 0640,
};


struct FileInfo {
    char path[MAX_PATH_LENGTH];
    char filename[MAX_FILENAME_LENGTH];
    int pid;
    int exists;
};
typedef struct FileInfo FileInfo;

// ----------------------- FIN DE ESTRUCTURAS DE DATOS UTILIZADAS --------------------------------------------------------------------------------------


// -------------------- GESTIÓN DE ERRORES DEL PROGRAMA -----------------------------------------------------------------------------------------

int
malloc_check(char *line) {

    if (line == NULL) {
        err(EXIT_FAILURE, "Memory asignation for path failed."); // Mensaje de error con información adicional
    } 
    return 0;
}

int
file_creation_check(int fd) {

    if (fd < 0) {
        printf("ERROR");
        err(EXIT_FAILURE, "The file doesn't exists."); // Mensaje de error con información adicional
    } 
    return 0;
}


// ---------------------- FIN DE GESTIÓN DE ERRORES DEL PROGRAMA------------------------------------------------------------------------------

char *
filename_extractor(char *line) {

    char *filename = (char *)malloc(FILENAME_BUFFER_SIZE); // Reservamos memoria para el nombre del archivo
    int filename_crt = 0;
    int last_bar_position;
    char extension[6] = ".sha1";

    
    for (int i=0; (line[i] != '\0'); i++) {
        if (line[i] == '/') {
            last_bar_position = i;
        }
    }

    
    for (int b=last_bar_position+1; (line[b] != '\0'); b++ ) {
        filename[filename_crt] = line[b];
        filename_crt++;
    }

    // Añadimos extensión .sha1
    for (int c = 0; (extension[c]) != '\0'; c++) {
        filename[filename_crt] = extension[c];
        filename_crt++;
    }
    
    return filename;
    

}

char *
clean_line(char *line) {
    for (int i=0; (line[i] != '\0'); i++) {
        if (line[i] == '\n') {
            line[i] = '\0';
        }
    }
    return line;
}



void 
sha_create(char *line, int *status, int fd) {

    
    int file_pid;
    switch (file_pid = fork()) {
        case -1:
            err(1, "fork failed");
        case 0:
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("Error to STDOUT redirection.");
                exit(1);
            }

            int exists = open(line, READ_PERMISSION);
            if (exists < 0) {
                *status = 1;
                printf("ERROR");
                exit(1);
            }

            if (exists >= 0) {
                // Cerrar el descriptor de archivo no necesario
                close(fd);

                // Redirigir la salida estándar al archivo
                if (dup2(exists, STDIN_FILENO) == -1) {
                    perror("Error to STDIN redirection.");
                    close(exists);
                    exit(1);
                }

                // Ejecutar sha1sum
                execl("/usr/bin/sha1sum", "sha1sum", NULL); // Ruta a sha1sum puede variar
                perror("execl failed");
                close(exists);
                exit(1);
            }
            close(exists);
            exit(0);
        default:
            int child_status;

            if (wait(&child_status) == -1) {
                perror("wait");
                // Manejar el error aquí, por ejemplo, retornar un código de error o salir del programa
            } else {
                if (WIFEXITED(child_status)) {
                    // El proceso hijo ha terminado normalmente
                    printf("ESto vale el status %d \n", WEXITSTATUS(child_status));
                    if (WEXITSTATUS(child_status) > 0) {
                        *status = WEXITSTATUS(child_status);
                    }
                    
                } else {
                    // El proceso hijo no terminó normalmente (por ejemplo, fue interrumpido)
                    // Puede manejar este caso dependiendo de su lógica de programa
                }
            }
                
    }

   
}

void
create_files(char *line, int *status) {
    char *filename = filename_extractor(line);

    // CREAMOS EL ARCHIVO NUEVO CON EL NOMBRE DEL ARCHIVO A ENCRIPTAR
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0640);
    file_creation_check(fd);
    
    sha_create(line, status, fd);
        
    
    
    
    // CERRAMOS ARCHIVO A ENCRIPTAR
    close(fd);
    free(filename);

}

void
read_lines(int *status) {

    char *line = (char *)malloc(LINE_BUFFER_SIZE); // Hacemos una asignación inicial de memoria de 256 caracteres por linea
    
    malloc_check(line);
    
    while (fgets(line, LINE_BUFFER_SIZE, stdin) != NULL) { //Cada vez que llamemos a fgets, se sobreescribirá line

        // EJECUTAMOS EL COMANDO SI RETORNA ERROR ES QUE EL ARCHIVO NO EXISTE POR LO QUE NO HABRA QUE CREAR UNO NUEVO
        line = clean_line(line); //Limpiamos line porque alfinal tiene un '\n'
        //printf("line: %s \n", line);
        create_files(line, status);
        

    }

    if (!feof(stdin)) {
        // Llegamos al final de la entrada estándar
        errx(EXIT_FAILURE, "eof not reached");
        
    } 


    free(line);
    
}


int
main(int argc, char *argv[])
{
    int status = 0;

    if (argc > 1){
	    err(EXIT_FAILURE, "No arguments needed.");
    }

    read_lines(&status);
    exit(status);
	return 0;

}