#include <stdio.h>
#include <unistd.h>

int main() {
    // Ruta al comando y sus argumentos
    char *path = "/bin/grep";    // Ruta al comando grep
    char *arg1 = "texto_buscado archivo.txt"; // Argumento 1: texto a buscar
    //char *arg2 = "archivo.txt";   // Argumento 2: nombre del archivo

    // Usar execl para ejecutar el comando grep con múltiples argumentos
    execl(path, path, arg1 , NULL);

    // Esta línea se ejecutará solo si execl falla
    perror("execl");
    return -1;
}