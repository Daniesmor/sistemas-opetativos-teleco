#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_FILENAME_LEN 256
#define MAX_DIGITS 3

void args_err() {
    fprintf(stderr, "usage: mysplit N file\n");
    exit(EXIT_FAILURE);
}

void split_file(int chunk_size, char *filename) {
    int file_descriptor = open(filename, O_RDONLY);
    if (file_descriptor == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    off_t total_bytes = lseek(file_descriptor, 0, SEEK_END);
    lseek(file_descriptor, 0, SEEK_SET);

    char buffer[chunk_size + 1];
    int file_count = 0;

    while (total_bytes > 0) {
        ssize_t bytes_read = read(file_descriptor, buffer, chunk_size);
        if (bytes_read == -1) {
            perror("Error al leer el archivo");
            exit(EXIT_FAILURE);
        }

        buffer[bytes_read] = '\0';

        char new_filename[MAX_FILENAME_LEN];
        snprintf(new_filename, MAX_FILENAME_LEN, "%03d%s", file_count++, filename);

        int new_file_descriptor = open(new_filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
        if (new_file_descriptor == -1) {
            perror("Error al crear el archivo");
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_written = write(new_file_descriptor, buffer, bytes_read);
        if (bytes_written == -1) {
            perror("Error al escribir en el archivo");
            exit(EXIT_FAILURE);
        }

        close(new_file_descriptor);

        total_bytes -= bytes_read;
    }

    close(file_descriptor);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        args_err();
    }

    int chunk_size = atoi(argv[1]);
    if (chunk_size <= 0) {
        args_err();
    }

    char *filename = argv[2];

    split_file(chunk_size, filename);

    return 0;
}