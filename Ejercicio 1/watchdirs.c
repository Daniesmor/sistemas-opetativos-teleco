#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>


void imprimirFecha() {
    var fecha = new date(1);

}

int
main (int argc, char *argv[])
{
    pid_t father_pid = getpid();
    pid_t child_pid;
    while (true) {
        
        imprimirFecha();
        printf("Watchdirs started (PID: %d)\n", father_pid);
        int execl(const char *path, const char *arg);
        sleep(3);
    }
    for (int i = 1; i < argc; i++) {
        child_pid = fork();

        if (child_pid == -1) {
            perror("fork"); // Manejar error de creación de proceso hijo
            return 1;
        }

        if (child_pid == 0) {
            // Código del proceso hijo
            printf("Soy el proceso hijo (PID: %d)\n", getpid());
        } else {
            // Código del proceso padre
            printf("Soy el proceso padre (PID: %d)\n", getpid());
        }

    }

}