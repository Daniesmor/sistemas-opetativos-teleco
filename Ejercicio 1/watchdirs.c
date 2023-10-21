#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>


void
list_directory() {
    execl("/bin/ls", "ls" "-1", NULL); 
}

void
print_date() {
    pid_t date_pid = fork();

    if (date_pid == -1) {
        perror("fork");
        exit(1);
    } 
    if (date_pid == 0) {
        execl("/bin/date","date", NULL); 
        exit(1);
    }
}

void 
create_process(int i, char *argv[]) {
    pid_t child_pid = fork();
    printf("%d", i);

    if (child_pid == -1) {
        perror("fork"); // Manejar error de creación de proceso hijo
        exit(1);
    }

    if (child_pid == 0) {
        chdir(argv[i]);
        list_directory();
        exit(1); 
    } 
}

void list_directories(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        create_process(i, argv);
    }
}


int
main (int argc, char *argv[])
{
    pid_t father_pid = getpid();
    printf("Watchdirs started (PID: %d)\n", father_pid);
    
    while (true) {
        print_date();

        list_directories(argc, argv);
        
        sleep(1);
    }
}