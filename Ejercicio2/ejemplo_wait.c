#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>



void
print_date()
{
	pid_t date_pid = fork();

	if (date_pid == -1) {
		fork_failed();
	}
	if (date_pid == 0) {
        printf("SOY EL HIJO %d \n", date_pid);
		execl("/bin/date", "date", NULL);
		exit(42);
	}
    else {
        printf("HA EMPEZAADO EL PADRE, EL PID DEL DATE ES %d \n", date_pid);

        int status;
        pid_t child_pid_date = wait(&status); //AQUI EL PADRE SE BLOQUEA HASTA QUE EL HIJO HACE ALGO, Y LO QUE RETORNARA SERÁ EL PID DEL PRIMER HIJO QUE TERMINE Y TENDRA QUE SER MAYOR IGUAL A 0

        if (child_pid_date > 0) {
            if (WIFEXITED(status)) {
                printf("Ya se ha impreso la fecha con PID %d terminado con estado: %d\n", child_pid_date, WEXITSTATUS(status)); //YA SE HA IMPRESO LA FECHA

            } else {
                printf("El proceso hijo con PID %d terminó de manera anormal.\n", child_pid_date);
            }
        } 

    }
}


int
main(int argc, char *argv[])
{	
	pid_t father_pid = getpid();
	
	printf("Watchdirs started (PID: %d)\n", father_pid);
	
	while (true) {
		print_date();

        printf("Ya estamos en main despues de date \n");
       
		
		sleep(1);

	}

	return 0;
}