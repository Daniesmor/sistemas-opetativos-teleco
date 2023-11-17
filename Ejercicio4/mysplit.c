#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

enum {
	rfile_permissions = 0666,
	wfile_permissions = 0664,
};

void
args_err()
{
	printf("usage: mysplit N file \n");
	exit(1);
}

int
leer_archivo(int size_bytes, char *file, int file_descriptor, char *buffer)
{

	int bytes_readed = read(file_descriptor, buffer, size_bytes);

	if (bytes_readed == -1) {
		perror("HA HABIO UN ERROR EN LA LECTURA \n");
		exit(1);
	}

	return bytes_readed;
}

void
set_name(int wfile_counter, char *buffer_wfile, int wfile_name_buffer_size,
	 char *filename)
{
	if (wfile_counter < 10) {
		snprintf(buffer_wfile, wfile_name_buffer_size, "00%d%s",
			 wfile_counter, filename);
	} else if (wfile_counter < 100) {
		snprintf(buffer_wfile, wfile_name_buffer_size, "0%d%s",
			 wfile_counter, filename);
	} else {
		snprintf(buffer_wfile, wfile_name_buffer_size, "%d%s",
			 wfile_counter, filename);
	}
}

int
open_files(char *file, int mode)
{
	int file_descriptor;

	if (mode == 1) {
		file_descriptor = open(file, rfile_permissions);	//PERMISOS DE SOLO LECTUA
	} else {
		file_descriptor = open(file, O_CREAT | O_WRONLY | O_TRUNC, wfile_permissions);	//PERMISOS DE LECTURA Y ESCRITURA
	}

	if (file_descriptor == -1) {
		args_err();
	}

	return file_descriptor;
}

void
close_files(int file_descriptor)
{
	int close_id = close(file_descriptor);

	if (close_id == -1) {
		perror("Close failed");
		exit(1);
	}
}

void
write_files(int wfile_descriptor, char *read_buffer, int bytes_readed)
{
	int bytes_writed = write(wfile_descriptor, read_buffer, bytes_readed);

	if (bytes_writed == -1) {
		perror("Write failed \n");
		exit(1);
	}
}

void
create_smalls(char *file, char *read_buffer, int bytes_readed,
	      int *wfile_name_counter)
{

	char wfile_name_buffer[10];

	set_name(*wfile_name_counter, wfile_name_buffer,
		 sizeof(wfile_name_buffer), file);
	int wfile_descriptor = open_files(wfile_name_buffer, 0);

	// ESCRIBIMOS CONTENIDO DE CADA FICHERO PEQUEÑO

	write_files(wfile_descriptor, read_buffer, bytes_readed);

	// CERRAMOS FICHERO PEQUEÑO
	close_files(wfile_descriptor);
}

void
read_write_files(int size_bytes, char *file, int rfile_descriptor)
{
	int bytes_readed = 1;	//LE DEFINIMOS COMO1 PARA QUE PUEDA ENTRAR AL WHILE
	char read_buffer[256];
	int wfile_name_counter = 0;

	while (bytes_readed != 0) {

		// LEEMOS EL FICHERO
		bytes_readed = read(rfile_descriptor, read_buffer, size_bytes);
		if ((bytes_readed <= size_bytes) && (bytes_readed != 0)) {

			// CREAMOS FICHEROS MAS PIQUEÑOS
			create_smalls(file, read_buffer, bytes_readed,
				      &wfile_name_counter);
			wfile_name_counter++;

		}

	}
}

void
split_files(int size_bytes, char *file)
{

	// ABRIMOS EL FICHERO PARA LECTRA fd=file descriptor descritpor de ficharo
	int rfile_descriptor = open_files(file, 1);

	// LEEMOS Y MANIPULAMOS EL FICHERO
	read_write_files(size_bytes, file, rfile_descriptor);

	// CERRAMOS EL FICHERO DE LECTURA
	close_files(rfile_descriptor);

}

void
split_data(int argc, char *argv[])
{
	if (argc < 3) {
		args_err();
	} else {
		int size_bytes = atoi(argv[1]);
		char *file = argv[2];

		if (size_bytes <= 0) {
			args_err();
		}

		split_files(size_bytes, file);
	}

}

int
main(int argc, char *argv[])
{

	split_data(argc, argv);
	return 0;

}
