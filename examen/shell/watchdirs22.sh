

if [ $# = 0 ]; then
    echo "You must at least one dir."
fi



while (true);
do
    empty_dirs=0
    echo $( date )

    for dir in $@;
    do

        #exist=$( [ -d "${dir}" ] && echo existe )
        #echo "${exist}"

        if [ -d "${dir}" ]; then
            
            dir_content=$( ls "${dir}" )
            echo "${dir_content}"

        else

            empty_dirs=$(( empty_dirs + 1 ))

        fi

        if [ ${empty_dirs} = $# ]; then
            echo "no dirs"
        fi


    done

    sleep 1

done



