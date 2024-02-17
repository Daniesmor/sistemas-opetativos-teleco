#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum {
	Distance = 'a' - 'A',
};

void
capitalizer(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		for (int b = 0; argv[i][b] != '\0'; b++) {
			if (argv[i][b] >= 'a' && argv[i][b] <= 'z') {
				argv[i][b] = argv[i][b] - Distance;
			}
		}
	}
}

void
insertion_sort(int argc, char *argv[])
{
	for (int i = 2; i < argc; i++) {
		char *z = argv[i];
		int j = i - 1;

		while (j >= 1 && strcmp(z, argv[j]) < 0) {
			char *a = argv[j];

			argv[j] = argv[j + 1];
			argv[j + 1] = a;
			j--;
		}
	}
}

void
selection_sort(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		for (int j = i + 1; j < argc; j++) {
			char *z = argv[i];

			if (strcmp(argv[i], argv[j]) > 0) {
				argv[i] = argv[j];
				argv[j] = z;
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	if (argc == 1) {
		printf("Error: No arguments\n");
		exit(-1);
	} else {
		capitalizer(argc, argv);
		insertion_sort(argc, argv);
		for (int i = 1; i < argc; i++) {
			printf("%s\n", argv[i]);

		}
	}

	return 0;
}
