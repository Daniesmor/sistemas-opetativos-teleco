El siguiente programa:
#include <stdio.h>
#include <stdlib.h>

enum {
	NItems = 32768,
};

int
main(int argc, char *argv[])
{
	char *array;
	array = (char *)malloc(NItems);
	array[16] = 'r';
	printf("%ld\n", sizeof(array));
	free (array);
	exit (EXIT_SUCCESS);
}
