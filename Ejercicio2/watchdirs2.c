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

		execl("/bin/date", "date", NULL);
		exit(1);
	} else {
		int status;

		wait(&status);
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
		pid_t child_pid = fork();

		if (child_pid == -1) {
			fork_failed();
		}

		if (child_pid == 0) {

			if (chdir(argv[i]) != 0) {
				argv[i] = NULL;
				exit(1);
			} else {
				list_directory();
				exit(0);
			}

		} else {
			int status;
			pid_t child_pid_process = wait(&status);

			if (child_pid_process > 0) {
				if (WIFEXITED(status)) {
					empty_folders =
					    empty_folders + WEXITSTATUS(status);
					if (empty_folders == argc - 1) {
						printf("No dirs \n");
						exit(1);
					}
				} else {
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
		} else {
			int status;

			wait(&status);
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

		list_directories(argc, argv);

		sleep(1);

	}

	return 0;
}
