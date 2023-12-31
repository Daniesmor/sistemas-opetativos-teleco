#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void
compare_files(int argc, char *argv[])
{

	int noreps = 1;

	for (int i = 1; i < argc; i++) {
		int repetidos = 0;

		for (int j = 1; j < argc; j++) {

			pid_t cmp_pid = fork();

			if (cmp_pid == -1) {
				printf("Fork failed \n");
				exit(1);
			}
			if (cmp_pid == 0) {
				execl("/bin/cmp", "cmp", "-s", argv[i], argv[j],
				      NULL);
				exit(2);
			} else {
				int status;

				wait(&status);
				if (argv[i] != argv[j]) {
					if (WEXITSTATUS(status) == 0) {
						repetidos = repetidos + 1;

					}
					if (WEXITSTATUS(status) == 2) {
						printf
						    ("usage: uniqfiles [files ...] \n");
						exit(2);
					}
				}

			}
		}

		if (repetidos == 0) {
			printf("%s \n", argv[i]);
		} else {
			noreps = 1;
		}
	}

	if (noreps == 1) {
		exit(0);
	}

}

void
compare(int argc, char *argv[])
{
	if (argc <= 2) {

		exit(0);

	} else {
		compare_files(argc, argv);
	}
}

int
main(int argc, char *argv[])
{

	compare(argc, argv);

	return 0;
}
