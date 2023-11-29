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

int 
collect_child_status(int *status) {
    int child_status;
    int new_status = *status;

    if (wait(&child_status) == -1) {
        // Manejar el error aquí, por ejemplo
        err(EXIT_FAILURE, "The child process has an error");
                
    } 
    else {
        if (WIFEXITED(child_status)) {
            // El proceso hijo ha terminado normalmente
            if (WEXITSTATUS(child_status) > 0) {
                new_status = WEXITSTATUS(child_status);
            }
                    
        } else {
            // El proceso hijo no terminó normalmente (por ejemplo, fue interrumpido)
            err(EXIT_FAILURE, "The child process finalized with an error");
                    
        }
    }
    return new_status;
    
}

void
exist_file_checker(int exists){
    if (exists < 0) {  //Si el archivo no existe, se sale en estatus de error.
        //*status = 1;
        printf("ERROR");
        exit(EXIT_FAILURE);
    }
}


void 
sha_create(char *line, int *status, int fd) {

    
    int file_pid;
    switch (file_pid = fork()) {
        case -1:
            err(1, "fork failed");
        case 0:
            // STDOUT_FILENO (nombre simbolico) -> 1
            if (dup2(fd, STDOUT_FILENO) == -1) {
                err(EXIT_FAILURE, "Error to STDOUT redirection.");
            }

            int exists = open(line, READ_PERMISSION); //Intentamos abrir el fichero que queremos encriptar
            exist_file_checker(exists); 


            if (exists >= 0) {
                // Cerrar el descriptor de archivo no necesario
                close(fd);
                // exists es el descriptor de fichero que contiene la info que se quiere encriptar
                // Redirigir la salida estándar al archivo STDIN_FILENO (nombre simbolico) -> 0 
                if (dup2(exists, STDIN_FILENO) == -1) {
                    close(exists);
                    err(EXIT_FAILURE, "Error to STDIN redirection.");
                }

                // Ejecutar sha1sum (Lo que está ocurriendo sha1sum < exists)
                execl("/usr/bin/sha1sum", "sha1sum", NULL); 
                close(exists);
                err(EXIT_FAILURE, "execl failed");
            }
            close(exists);
            exit(EXIT_SUCCESS);
        default:
            *status = collect_child_status(status); //Esta función recolecta el estado de salida del hijo 
            
                
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