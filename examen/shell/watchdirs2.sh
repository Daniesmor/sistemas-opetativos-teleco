#!/bin/sh


if [ $# = 0 ]; then

    echo "You must enter args"
    exit

fi



while true
do
    empty_dirs=0
    date=$(date)
    echo "${date}"

    for dir in $@;
    do

        #echo "${dir}"
        # COMPROBAMOS SI EXISTE EL DIRECTIORIO
        if [ -d ${dir} ]; then
            # SI EXISTE ACCEDEMOS AL DIRECTORIO A TRAVES DE UNA SUBSHELL Y LISTAMOS SU CONTENIDO
            dir_content=$(ls ${dir})
            echo "${dir_content}"
            
            #num_files=$(ls ${dir} | wc -l)
            #echo "num linea ${num_files}"
            #echo "Contenido de ${dir}:"
            
        else
            empty_dirs=$((empty_dirs+1))  

        fi


        

        #existes 

    done
    #echo "empty "${empty_dirs}""

    if [ $empty_dirs = $# ]; then
            echo "No dirs"
            break
            
    fi
    


    sleep 1


done
