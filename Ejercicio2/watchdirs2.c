#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


void
list_directory()
{
	execl("/bin/ls", "ls", "-1", NULL);
	perror("An error with ls command has ocurred \n");
	exit(1);
}

void
fork_failed()
{
	perror("An error has ocurred with the fork");
	exit(1);
}

void
print_date()
{
	pid_t date_pid = fork();

	if (date_pid == -1) {
		fork_failed();
	}
	if (date_pid == 0) {
        //printf("SOY EL HIJO %d \n", date_pid);
		execl("/bin/date", "date", NULL);
		exit(42);
	}
    else {
        //printf("HA EMPEZAADO EL PADRE, EL PID DEL DATE ES %d \n", date_pid);

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

void
unkown_path(int i)
{
	printf("The path %d doesn't exist \n", i);
    if (i == 2) {
        printf("Se produjo un error.\n");
        exit(1);
    }
	exit(1);
}

void
list_process(int argc, char *argv[])
{
    int empty_folders = 0;

    for (int i = 1; i < argc; i++) {
        printf("Estamos en el directorio %d \n", i);
        pid_t child_pid = fork();

        if (child_pid == -1) {
            fork_failed();
        }

	    if (child_pid == 0) {

            if (chdir(argv[i]) != 0) {
                argv[i] = NULL;
                empty_folders = empty_folders +1;
                //printf("Valor de emptyfolders %d", empty_folders);
                if (empty_folders == argc) {
                    printf("EStmamos jodias");
                }
                //printf("valor completo argv %s \n", argv[i]);
                //unkown_path(i); NO HACE FALTA TENERLO EN CUENTA 
            } else {
                list_directory();
            }
            exit(1);
	    }
        else {
            int status;
            pid_t child_pid_process = wait(&status); 

            if (child_pid_process > 0) {
                if (WIFEXITED(status)) {
                    //printf("Ya se ha ejecutado list_process con PID %d terminado con estado: %d\n", child_pid_process, WEXITSTATUS(status)); 
                } else {
                    //printf("El proceso hijo con PID %d terminó de manera anormal.\n", child_pid_process);
                }
            } 
        }   
    

	}
	
    

}

void
list_directories(int argc, char *argv[])
{

    
	if (argc == 1) {
		pid_t child_pid = fork();
		
		if (child_pid == -1) {
			fork_failed();
		}

		if (child_pid == 0) {
			list_directory();
		}
        else {
            int status;
            pid_t child_pid_directories = wait(&status); 

            if (child_pid_directories > 0) {
                if (WIFEXITED(status)) {
                    //printf("Ya se ha ejecutado LS con PID %d terminado con estado: %d\n", child_pid_directories, WEXITSTATUS(status)); 
                } else {
                    //printf("El proceso hijo con PID %d terminó de manera anormal.\n", child_pid_directories);
                }
            } 
        }

	} else {

        list_process(argc, argv);


	}
	
}

int
main(int argc, char *argv[])
{	
	pid_t father_pid = getpid();
	
	printf("Watchdirs started (PID: %d)\n", father_pid);
	
	while (true) {
		print_date();

        printf("------------------- YA ESTAMOS EN EL MAIN DESPUES DE DATE ----------------  \n");
        list_directories(argc, argv);
		printf("-------------------SE HA TERMINADO LA VUELTA ---------------- \n");
		sleep(1);

	}

	return 0;
}