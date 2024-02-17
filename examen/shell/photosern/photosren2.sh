#!/bin/sh


if [ $# = 0 ]; then

    echo "You must enter args"
    exit

fi

if [ $# != 1 ]; then

    echo "You must enter only one arg"
    exit

fi


date=$(date +"%d_%b_%Y")
mkdir "${date}"


count=0
for image in $( ls $1 )
do
    
    is_image=$( file -b "${image}" | grep "image" | wc -l )
    file=$(file -b "${image}")
    #echo "${is_image}"
    if [ ${is_image} = 1 ];
    then
        echo "${image}"
        count_format=$( printf "%03d" "${count}")
        echo "${count_format}"
        destination=$( echo "${date}"/"${count_format}" )
        mv "${image}" "${date}"/"${count_format}"
        count=$(( count + 1 ))
    fi
done