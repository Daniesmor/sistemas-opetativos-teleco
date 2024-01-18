gcc -Wall -Wshadow -Wvla -g -c proyect.c
gcc -g -o proyect proyect.o
valgrind --leak-check=yes ./proyect



