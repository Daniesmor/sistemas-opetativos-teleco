#include <stdio.h>
#include <string.h>

void capitalizer(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        for (int b = 0; argv[i][b] != '\0'; b++) {
            if (argv[i][b] >= 97 && argv[i][b] <= 122) {
                argv[i][b] = argv[i][b] - 32;
            }
        }
    }
}

void sort(int argc, char *argv[]) {
    for (int i = 1; i < argc - 1; i++) {
        for (int j = i +1; j < argc; j++)
            if (strcmp(argv[i], argv[j]) > 0) {
                char *z = argv[i];
                argv[i] = argv[j];
                argv[j] = z;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Error: No arguments\n");
    } else {
        capitalizer(argc,argv);
        sort(argc,argv);
        for (int i = 1; i < argc; i++) {
            printf("%s\n", argv[i]);
            
        }
    }

    return 0;
}
