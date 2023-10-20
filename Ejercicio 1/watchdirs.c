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
    int father_pid = getpid();
    while (true) {
        
        imprimirFecha();
        printf("Watchdirs started (PID: %d)\n", father_pid);
        int execl(const char *path, const char *arg, ...);
        sleep(3);
    }

}