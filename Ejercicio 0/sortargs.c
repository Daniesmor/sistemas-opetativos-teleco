#include <stdio.h>
#include <string.h>

void
capitalizer(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		for (int b = 0; argv[i][b] != '\0'; b++) {
			if (argv[i][b] >= 'a' && argv[i][b] <= 'z') {
				argv[i][b] = argv[i][b] - 32;
			}
		}
	}
}

void
sort(int argc, char *argv[])
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

int
main(int argc, char *argv[])
{
	if (argc == 1) {
		printf("Error: No arguments\n");
		return 1;
	} else {
		capitalizer(argc, argv);
		sort(argc, argv);
		for (int i = 1; i < argc; i++) {
			printf("%s\n", argv[i]);

		}
	}

	return 0;
}
