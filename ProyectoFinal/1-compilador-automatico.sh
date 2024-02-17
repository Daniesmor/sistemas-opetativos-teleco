#gcc -Wall -Wshadow -Wvla -fprofile-arcs -ftest-coverage -o -g -c shell shell.c
#gcc -g -o shell shell.o
gcc -fprofile-arcs -ftest-coverage -o shell shell.c
valgrind --leak-check=yes ./shell
