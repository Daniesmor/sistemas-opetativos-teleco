//SUPONER QUE HAY MENOS DE 1000 FICHERS
// USAR READ Y WRITE
//ABRIR USAR EL ARCHIVO Y CERRAR
//hay que cambiar d edirectorio??
// indagar mas sobre descriptores de ficheros, salida estandar de errores???
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


void 
set_name(int wfile_counter, char *buffer_wfile, int wfile_name_buffer_size) 
{
    if (wfile_counter < 10) 
    {
        snprintf(buffer_wfile, wfile_name_buffer_size, "00%d", wfile_counter);
    } else if (wfile_counter < 100) {
        snprintf(buffer_wfile, wfile_name_buffer_size, "0%d", wfile_counter);
    } else {
        snprintf(buffer_wfile, wfile_name_buffer_size, "%d", wfile_counter);
    }
}

void
split_proccess()
{


    int permissions = 0444;


    int wfile_counter = 979;
    char wfile_name_buffer[10000];

    set_name(wfile_counter, wfile_name_buffer, sizeof(wfile_name_buffer));
	// ABRIMOS EL FICHERO PARA LECTRA fd=file descriptor descritpor de ficharo
    printf("%s \n", wfile_name_buffer); // Nombre del archivo
    int file_descriptor = open(wfile_name_buffer, O_CREAT | O_WRONLY | O_TRUNC, permissions);
    
    if (file_descriptor == -1) {
        perror("Error al abrir el archivo");
        exit(1);
    }



}






int
main(int argc, char *argv[]) 
{

    split_proccess();
    return 0;

}







void read_write_files(int size_bytes, char *file, int rfile_descriptor) {
    int buffer_size = 1024; // Tamaño inicial del búfer más pequeño
    char *read_buffer = malloc(buffer_size);

    if (read_buffer == NULL) {
        perror("Error al asignar memoria");
        exit(1);
    }

    int wfile_name_counter = 0;
    int bytes_readed = 0;

    do {
        bytes_readed = read(rfile_descriptor, read_buffer, size_bytes);

        if (bytes_readed < 0) {
            perror("Error en la lectura del archivo");
            free(read_buffer);
            exit(1);
        }

        if ((bytes_readed <= size_bytes) && (bytes_readed != 0)) {
            // Aquí se debería escribir la lógica para crear archivos más pequeños
             create_smalls(file, read_buffer, bytes_readed, &wfile_name_counter);
             wfile_name_counter++;
        }
    } while (bytes_readed > 0);

    free(read_buffer);
}