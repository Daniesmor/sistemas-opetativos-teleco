#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>

// ----------------------- ESTRUCTURAS DE DATOS UTILIZADAS --------------------------------------------------------------------------------------

enum {
    MAX_PATH_LENGTH = 256,
};

struct FileInfo {
    char *path[MAX_PATH_LENGTH];
    int pid;
    int exists;
};


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
file_creation(int fd) {

    if (fd != 0) {
        err(EXIT_FAILURE, "File not created."); // Mensaje de error con información adicional
    } 
    return 0;
}




// ---------------------- FIN DE GESTIÓN DE ERRORES DEL PROGRAMA------------------------------------------------------------------------------

void 
sha_create(char *line) {
    int file_pid;
    switch (file_pid = fork()) {
	case -1:
		err(EXIT_FAILURE, "fork failed");
	case 0:
        printf("line en sha %s", line);
		execl("/bin/sha1sum", "sha1sum", line, NULL);
		err(EXIT_FAILURE, "exec failed");
	}
	
}

void
read_lines() {

   
    char *line = (char *)malloc(MAX_PATH_LENGTH * sizeof(char)); // Hacemos una asignación inicial de memoria de 256 caracteres por linea
    malloc_check(line);
    printf("line en read_lines %p", &line);
    while (fgets(line, sizeof(line), stdin) != NULL) { //Cada vez que llamemos a fgets, se sobreescribirá line
        printf("line: %s \n", line);
        
        // EJECUTAMOS EL COMANDO SI RETORNA ERROR ES QUE EL ARCHIVO NO EXISTE POR LO QUE NO HABRA QUE CREAR UNO NUEVO
        sha_create(line);

        // CREAMOS EL FICHERO QUE CONTENDRÁ EL SHA
        int fd = open(line, O_CREAT, 0644);
        file_creation(fd);

    }

    free(line); // Liberamos la memoria

}

void
split_data() {

    // Creamos ficheros 
    //FileInfo *file[] = (Fileinfo *)malloc(sizeof(FileInfo));
    //leer lineas entrada estnadar, por cada linea cresmos un fichero

    read_lines();

    printf("hola");
    
}



int
main(int argc, char *argv[])
{

	split_data();
	return 0;

}
