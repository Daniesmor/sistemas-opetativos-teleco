#!/bin/sh

if [ $# = 0 ]; then

    echo "You must enter args".
    exit

fi


if [ $# != 1 ]; then

    echo "You must enter only one args".
    exit

fi


date=$(date +"%d_%b_%Y");
$(mkdir -p "${date}");

## Creamos la carpeta
#echo "${date}"

count=0
for files in $(ls $1)
do

    format=$(file -i "${files}" | grep "image" | wc -l) # IMPRIMIMOS CADA FORMATO

    if [ $format = 1 ]; then
        count_format=$(printf "%03d" "${count}")
        destination_dir=$(echo "${date}"/"${count_format}")
        #echo "${destination_dir}"
        $(mv "${files}" "${destination_dir}")
        count=$(( count + 1 ));
    fi

    #echo "${format}"

done