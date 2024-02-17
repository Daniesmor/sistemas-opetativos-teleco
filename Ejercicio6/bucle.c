

int child;

for (int numCommand=0; numCommand<NUM_PARAMS; numCommand++) {
    switch (child = fork()) { // ---------- HIJO 1
        case -1:
            err(EXIT_FAILURE, "Theres an error with the child proccess");
        case 0:
            if (numCommand > 0) { //SI NO ES EL PRIMER COMANDO, LA ENTRADA LA TIENE QUE LEER DEL COMANDO ANTERIOR
                dup2(pipes[numCommand-1][0], STDIN_FILENO); //Si no es el pirmer comando, deberá leer la entrada de la salida del anterior.
                close(pipes[numCommand-1][0]); //Como en dup2 ya se ha hecho el duplicado, podemos cerrar el pipe
                close(pipes[numCommand-1][1]); //Del pipe que lo precede, solo queremos leer la entrada, por lo tanto cerramos la escritura
    
            }
            if (numCommand < NUM_PARAMS -1 ) { //SI NO ES EL ULTIMO COMANDO, LA SALIDA DEBE SER ENVIADA AL SIGUIENTE COMANDO
                close(pipes[numCommand][0]); // COMO QUEREMOS ESCRIBIR EN EL PIPE QUE LO UNE CON EL SIG COMANDO, PODEMOS CERRAR EL EXTREMO DE LECTURA
                dup2(pipes[numCommand][1], STDOUT_FILENO); //REDIRIGIMOS LA SALIDA AL PIPE QUE LO UNE CON EL SIG COMANDO
                close(pipes[numCommand][1]); // COMO YA HEMOS HECHO EL DUPLICADO CON DUP2, LO PODEMOS BORRAR
            }
            // EJECUTAMOS EL COMANDO Y LO MANEJAMOS EN CASO DE ERROR
            execv(Commands[numCommand]->path, Commands[numCommand]->argumentos);
            printf("There's an error executing the comand %s. \n", Commands[numCommand]->nombre);
            exit(EXIT_FAILURE);
        default:
            // UNA VEZ QUE SE EJECUTE EL COMANDO, EL PADRE CERRARÁ LO QUE YA NO SE VA A VOLVER A USAR
            if (numCommand < NUM_PARAMS -1) {
                close(pipes[numCommand][1]); // Cierra el extremo de escritura del pipe actual
            }

    }
}

// Código del padre que espera a todos los hijos
for (int i = 0; i < NUM_PARAMS; i++) {
    int status;
    wait(&status);
}
