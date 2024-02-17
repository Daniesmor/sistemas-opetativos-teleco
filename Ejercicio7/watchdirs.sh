#!/bin/sh
num_dirs=$#

list_content() {
    dir="$1" #Las comillas se ponen para mantener los espacios en orden
    content=$(ls -1 "$dir")
    echo "$content"
}

count_empty_dirs() {
    count=0
    # Recorrer cada argumento pasado al script
    for arg in "$@"; do
        if [ -d "$arg" ]; then
            # Si el argumento es un directorio válido, listar su contenido
            content=$(list_content "$arg")
            if [ -z "$content" ]; then  #-z comprueba si la long del string es 0
                count=$((count + 1))
            fi
        else
            # Si el argumento no es un directorio válido, contar como directorio vacío
            count=$((count + 1))
        fi
    done
    echo "$count"
}

print_content() {
    for arg in "$@"; do
        if [ -d "$arg" ]; #-d comprueba si existe el directorio
        then
            list_content "$arg"
        fi
    done
}

list_dirs() {
    empty_dirs=0

    if [ $num_dirs -eq 0 ]; then  # Compara la variable con 0
        # Listar contenido en el directorio actual si no se proporcionan argumentos
        content=$(list_content ".")
        if [ -z "$content" ]; then
            echo "no dirs"
        else
            echo "$content"
        fi
    else
        empty_dirs=$(count_empty_dirs "$@") #Va a contar cuantos directorios hay vacios de todos los parametros

        # Si todos los argumentos son directorios vacíos, mostrar un mensaje y salir con un código de error
        if [ "$empty_dirs" -eq "$num_dirs" ]; then
            echo "no dirs"
            exit 1
        else
            print_content "$@" #Es necesario pasar los parametros comoa argumentos
                
        fi
    fi
}

while true; do
    date

    list_dirs "$@" #Es necesario pasar los parametros comoa argumentos

    sleep 1
done

exit 0