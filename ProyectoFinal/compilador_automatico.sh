gcc -Wall -Wshadow -Wvla -g -c shell.c
gcc -g -o shell shell.o
valgrind --leak-check=yes ./shell



