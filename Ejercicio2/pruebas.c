#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }

    if (pid == 0) {
        // Código del proceso hijo
        printf("Proceso hijo ejecutando...\n");
        exit(42); // Salir con un código de estado 42
    } else {
        // Código del proceso padre
        int status;
        pid_t child_pid = wait(&status);

        if (child_pid > 0) {
            if (WIFEXITED(status)) {
                printf("Proceso hijo con PID %d terminado con estado: %d\n", child_pid, WEXITSTATUS(status));
            } else {
                printf("El proceso hijo con PID %d terminó de manera anormal.\n", child_pid);
            }
        } else {
            perror("Esperando al proceso hijo");
        }
    }

    return 0;
}
