gcc -Wall -Wshadow -Wvla -g -c pruebas.c
gcc -g -o pruebas pruebas.o
valgrind --leak-check=yes ./pruebas
