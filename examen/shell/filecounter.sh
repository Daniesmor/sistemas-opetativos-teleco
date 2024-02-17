#!/bin/sh


if [ $# = 0 ]; then

    echo "You must enter dirs"

fi

current_dir="."
while [ -n current_dir ];
do 
    echo "current dir: " "${current_dir}"
    for file in $( ls "${current_dir}" );
    do
        
        is_dir=$( test -d "${file}" && echo 1 )
        #echo ${is_dir}
        if [ "${is_dir}" = "1" ]; 
        then # SI ES UNA CARPETA ACCEDEMOS A ELLA Y LISTAMOS SI CONTENTIDO
            #echo "${file}"
            dir_content=$( ls "${file}"/ )
            current_dir=$( echo ${file} )
            echo "${dir_content}"


       
        fi
        
        list_contant();

    done


done


