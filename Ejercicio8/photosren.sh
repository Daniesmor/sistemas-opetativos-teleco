
#!/bin/sh

validate_only_argument() {
    # VERIFICAMOS QUE EL ARGUMENTO ES UN DIRECTORIO
    if [ $# -ne 1 ]; then # SI SE HA DADO MAS DE 1 ARGUMENTO (DIRECTORIO), NO ES VALIDA
    echo "Introduce only one dir"
    exit 1
    fi

    # ASIGNAMOS EL DIRECTORIO A UNA VARIABLE
    directorio="$1"

    #COMPROBAMOS QUE ES UN DIRECTORIO VALIDO
    if [ ! -d "${directorio}" ]; then # EL ! SE UTILIZA PARA EVALUAR SI NO EXISTE EL DIR
        echo "Dir not valid"
        exit 1
    fi
}

create_new_dir() {
    # CREAMOS EL DIRECTORIO CON EL FORMATO DE FECHA ACTUAL
    fecha=$(date +"%d_%b_%Y") # $() SE QUEDARA CON LA SALIDA DE LO DE DENTRO
    new_dir="${directorio}/${fecha}" # ES RECOMENDABLE USAR LAS {} PARA EVITAR PROBLEMAS
    mkdir -p "${new_dir}" # LAS COMILLAS SON NECESARIAS, POR SI HUBIERA ESPACIOS EN BLANCO
}

rename_image_files() {
    # OBTENER Y RENOMBRAR ARCHVIOS DE IMAGEN
    contador=0 #CREAMOS EL CONTADOR DE ARCHIVOS
    for file in $(find "${directorio}" -type f) # find ME DA UNA LISTA CON TODOS LOS ARCHIVOS (-f)
    do
        
        type_file=$(file -b --mime-type "${file}") # EL COMANDO ME DA EL TIPO DE ARCHIVO, SEGUDIDO DE /extension

        type=$(echo "${type_file}" | cut -d '/' -f1) # -d es el opcion de delimitador y -f1, es que nos quedamos con laprimera palabra
        # AHORA QUE TENEMOS EL TIPO DE ARCHIVO
        if [ "${type}" = "image" ]; then # SI EL ARCHIVO ES DE TIPO IMAGEN...
            contador_formateado=$(printf "%03d" "${contador}") # FORMATEAMOS EL CONTADOR QUE TENGA 3 NUMEROS    
            extension=$(echo "${type_file}" | cut -d '/' -f2) # AHORA RECORTAMOS PARA QUEDARNOS CON LA EXTENSION
            new_name="${contador_formateado}.${extension}" # DEFINIMOS EL NUEVO NOMBRE
            mv "${file}" "${new_dir}/${new_name}" # MOVEMOS LOS ARCHIVOS
            contador=$((contador+1)) # INCREMENTAMOS EL CONTADOR
        fi
    done
}

main() {
    # COMO SE LLAMAN A LAS FUNCIOENS DENTRO DEL MAIN, LAS VARIABLES LOCALES DE CADA FUNCION SE COMPARTEN
    validate_only_argument "$@"
    create_new_dir
    rename_image_files
}

main "$@" # LLAMAMOS A LA FUNCION MAIN (ES NECEASRIO PASAR $@, SI NO NO FUNCIONA)