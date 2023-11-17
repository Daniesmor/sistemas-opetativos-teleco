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

int 
leer_archivo(int size_bytes,char *file, int file_descriptor, char *buffer)
{
    //int max_bytes_to_read = 1000;
    

    int bytes_readed = read(file_descriptor, buffer, size_bytes);

    printf("Se han leido %d bytes \n", bytes_readed);
    

    if (bytes_readed == -1) {
        perror("HA HABIO UN ERROR EN LA LECTURA \n");
        exit(1);
    }

    return bytes_readed;
}

void 
set_name(int wfile_counter, char *buffer_wfile, int wfile_name_buffer_size, char *filename) 
{
    if (wfile_counter < 10) 
    {
        snprintf(buffer_wfile, wfile_name_buffer_size, "00%d%s", wfile_counter, filename);
    } else if (wfile_counter < 100) {
        snprintf(buffer_wfile, wfile_name_buffer_size, "0%d%s", wfile_counter, filename);
    } else {
        snprintf(buffer_wfile, wfile_name_buffer_size, "%d%s", wfile_counter, filename);
    }
}

void
split_proccess(int size_bytes,char *file)
{
    printf("size: %d, file: %s\n", size_bytes, file);
    char buffer[100000];
    int lectura_completa = 1;
    int rfile_permissions = 0666;


    int wfile_counter = 0;
    char wfile_name_buffer[10000];
    int wfile_permissions = 0664;

	// ABRIMOS EL FICHERO PARA LECTRA fd=file descriptor descritpor de ficharo
    int file_descriptor = open(file, rfile_permissions);

    if (file_descriptor == -1) {
        perror("Error al abrir el archivo");
        exit(1);
    }
 
    
    // leemos archivo con read
    while (lectura_completa != 0)
    {

        
        lectura_completa = leer_archivo(size_bytes, file, file_descriptor, buffer);
        if (lectura_completa <= size_bytes) {
            //char wfile_name = sprintf(wfile_name, "%d",wfile_counter);

            // CREAMOS EL FICHERO PARA ESCRITURA
            set_name(wfile_counter, wfile_name_buffer, sizeof(wfile_name_buffer), file);
            printf("EL nombre es %zu \n", sizeof(wfile_name_buffer));
            int wfile_descriptor = open(wfile_name_buffer, O_CREAT | O_WRONLY | O_TRUNC, wfile_permissions);
            
            printf("Se ha creado el siguiente fd %d \n", wfile_descriptor);

            // ESCRIBIMOS EL FICHERO
            

            wfile_counter++;
            int bytes_writed = write(wfile_descriptor, buffer, lectura_completa);
            if (bytes_writed == -1) {
                perror("Hubo un fallo en la escritura \n");
                exit(1);
            }

            // CERRAMOS EL FICHERO
            int close_id = close(wfile_descriptor);
            if (close_id != -1) {
                //printf("cerrado correctamente \n");
            }
            //printf("\n");
            //printf("Se han escrito %d bytes \n", bytes_writed);
        }

    }


    // IMPRIMIMOS POR EL DESCRIPTOR DE FICHERO 0 (SALIDA ESTANDAR)
    /*
    printf("Se han leido %d bytes \n", lectura_completa);
    int bytes_writed = write(0, buffer, lectura_completa);
    printf("Se han escrito %d bytes \n", bytes_writed);
    */


}





void
split_func(int argc, char *argv[]) 
{
    int size_bytes = atoi(argv[1]);
    char *file = argv[2];


    split_proccess(size_bytes, file);

}


int
main(int argc, char *argv[]) 
{

    split_func(argc ,argv);
    return 0;

}