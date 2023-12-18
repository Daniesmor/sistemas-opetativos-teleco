#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

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
		execl("/bin/date", "date", NULL);
		exit(1);
	}
}

void
unkown_path(int i)
{
	printf("The path %d doesn't exist \n", i);
	exit(1);
}

void
list_process(int i, char *argv[])
{
	pid_t child_pid = fork();

	if (child_pid == -1) {
		fork_failed();
	}

	if (child_pid == 0) {
		if (chdir(argv[i]) != 0) {
			unkown_path(i);
		} else {
			list_directory();
		}
		exit(1);
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
	} else {
		for (int i = 1; i < argc; i++) {
			list_process(i, argv);
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

		list_directories(argc, argv);
		
		sleep(1);
	}

	return 0;
}