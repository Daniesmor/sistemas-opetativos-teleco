#!/bin/sh
 #Esto es para que no nos cuenta como parametro el nombre del programa
num_dirs=$#

list_dirs() {
    empty_dirs=0
    
    for arg in "$@"
    do
        if [ -d "$arg" ];
        then 
            content=$(ls -1 "$arg")
            if [ -z "$content" ];
            then
                empty_dirs=$(($empty_dirs + 1))
            else
                echo "$content"
            fi    
        else
            empty_dirs=$(($empty_dirs + 1))
        fi
    done
    
    if [ "$empty_dirs" -eq "$num_dirs" ];
    then
        echo "no dirs"
        exit 1
    fi
    
}

while true
do
    date

    list_dirs "$@"

    
    sleep 1
done

exit 0