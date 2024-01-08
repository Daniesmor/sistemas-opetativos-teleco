#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <sys/wait.h>
#include <string.h>

// ----------------------- PARAMETROS Y CONSTANTES ---------------------------------------------------------------------------------------------

enum {
	MAX_PATH_LENGTH = 256,
	MAX_FILENAME_LENGTH = 100,
	LINE_BUFFER_SIZE = MAX_PATH_LENGTH * sizeof(char),
	FILENAME_BUFFER_SIZE = MAX_FILENAME_LENGTH * sizeof(char),
	READ_PERMISSION = 0444,
	RW_PERMISSION = 0640,
    NUM_PARAMS = 3,
};

const char *builtin_cmds[] = {
	"cd",
	"=",
	"ifok", 
	"ifnot",
};

// ----------------------- FIN DE PARAMETROS Y CONSTANTES ---------------------------------------------------------------------------------------------

// ----------------------- ESTRUCTURAS DE DATOS UTILIZADAS --------------------------------------------------------------------------------------

struct Command {
	char *nombre;	// Campo para el nombre del comando
	char *path;
	int numArgumentos;	// Contador para llevar el seguimiento de la cantidad de argumentos
	char **argumentos;
	char *entrada;
	char *salida;
};

typedef struct Command Command;


struct Commands {

    int numCommands;
    Command **comandos;
	int background;

};

typedef struct Commands Commands;



// ----------------------- FIN DE ESTRUCTURAS DE DATOS UTILIZADAS --------------------------------------------------------------------------------------



// -------------------- GESTIÓN DE ERRORES DEL PROGRAMA -----------------------------------------------------------------------------------------

int
malloc_check(char *line)
{

	if (line == NULL) {
		err(EXIT_FAILURE, "Memory asignation for path failed.");	// Mensaje de error con información adicional
	}
	return 0;
}

void
memLocateFailed()
{
	err(EXIT_FAILURE,
	    "Error: Could not allocate memory for the argument.\n");
}


// ---------------------- FIN DE GESTIÓN DE ERRORES DEL PROGRAMA------------------------------------------------------------------------------

// ----------------------------- GESTION DEL STRUCT COMMAND ----------------------------------------------------------------------

void
initializerCommand(Command *cmd)
{

	cmd->nombre = NULL;
	cmd->argumentos = NULL;
	cmd->numArgumentos = 0;
	cmd->path = NULL;
	cmd->entrada = NULL;
	cmd->salida = NULL;

}

char **
reserve_args(Command *cmd) 
{
	// Reserva espacio para tener una lista con tantos punteros como argumentos halla.

	cmd->argumentos =
	    realloc(cmd->argumentos, (cmd->numArgumentos + 1) * sizeof(char *));
	return cmd->argumentos;

}


void
assignCommandName(Command *cmd, char *name)
{
	cmd->nombre = strdup(name);
}
// ----------------------------- FIN DE GESTION DEL STRUCT COMMAND ----------------------------------------------------------------------


// ---------------------------- GESTION DEL STRUCT COMMANDS ------------------------------------------------------------------------------------

void
initializerCommands(char *argv[], Commands *cmds)
{
	
	cmds->numCommands = 0;
	cmds->comandos = NULL;
	cmds->background = 0;

}


Command **
reserve_commands(Commands *cmds) //GESTIONA LA LISTA QUE CONTENDRÁ LOS PUNTEROS A LOS DIFERENTES COMANDOS
{

	cmds->comandos = realloc(cmds->comandos, (cmds->numCommands + 1) * sizeof(Command *)); //Reserva memoria para una lista de numCOmmands+1 punteros a Commands
	cmds->comandos[cmds->numCommands] = malloc(sizeof(Command)); // Reservamos memoria para el comando en cuestion
	initializerCommand(cmds->comandos[cmds->numCommands]); //Inicializamos el comando con los valores predetermiandos
	// RESERVAMOS LA MEMORIA DEL COMANDO EN CUESTION
	
	return cmds->comandos; //Retorna la dir de la lista que contiene las direcciones a los comandos

}

void
setLastArgumentNull(Command *cmd)
{

	if (reserve_args(cmd) == NULL) {
		// Manejar el error de asignación de memoria
		memLocateFailed();
		//free(argv_copy);
		return;
	}

	cmd->argumentos[cmd->numArgumentos] = NULL;	// Establecer el último elemento como NULL
}


// ----------------------------- FIN DE GESTION DEL STRUCT COMMANDS -----------------------------------------------------------------------------

// ------------------------------ ASIGNACION DE VARIABLES DE ENTORNO (COMANDO ESPECIAL) ---------------------------------------------------------------------------

void
variable_asig(Commands *cmds, char *token) // VAMOS A INTERPRETAR ESTE COMANDO COMO UN BUILT-IN PARA SIMPLIFICAR SU PROGRAMACION
{
	char *vars;
	char *saveptr;

	// ASIGNAMOS EL NOMBRE DEL COMANDO COMO "=", PARA PODER COTEJARLO CON LOS BUILTS-IN (lista)
	assignCommandName(cmds->comandos[cmds->numCommands], "=");
	vars = strtok_r(token, "=", &saveptr); //Token es una dir de memoria
	if (reserve_args(cmds->comandos[cmds->numCommands]) == NULL) {
                    // Manejar el error de asignación de memoria
            memLocateFailed();
            return;
    }

	// GUARDAMOS EL VALOR DE LA VARIABLE QUE QUEREMOS CREAR COMO ARGUMENTO 0
    cmds->comandos[cmds->numCommands]->argumentos[cmds->comandos[cmds->numCommands]->numArgumentos] = strdup(vars); 
    cmds->comandos[cmds->numCommands]->numArgumentos++;

	vars = strtok_r(NULL, " ", &saveptr); //vars es una dir de memoria
	if (reserve_args(cmds->comandos[cmds->numCommands]) == NULL) {
        // Manejar el error de asignación de memoria
        memLocateFailed();
        return;
    }

	// GUARDAMOS EL VALOR QUE LE VAMOS A DAR A LA VARIABLE QUE QUEREMOS CREAR COMO ARGUMENTO 1
    cmds->comandos[cmds->numCommands]->argumentos[cmds->comandos[cmds->numCommands]->numArgumentos] = strdup(vars);
	cmds->comandos[cmds->numCommands]->numArgumentos++;
}

void
variable_sustitution(Commands *cmds, char *token) // VAMOS A INTERPRETAR ESTE COMANDO COMO UN BUILT-IN PARA SIMPLIFICAR SU PROGRAMACION
{
	char *vars;
	char *saveptro;

	vars = strtok_r(token, "$", &saveptro);

	char* variable = getenv(vars);

	if (variable == NULL) {
		printf("error: var %s does not exist. \n", vars);
	} else
	{
		if (reserve_args(cmds->comandos[cmds->numCommands]) == NULL) {
			// Manejar el error de asignación de memoria
			memLocateFailed();
			return;
		}

		cmds->comandos[cmds->numCommands]->argumentos[cmds->comandos[cmds->numCommands]->numArgumentos] = strdup(variable);
		cmds->comandos[cmds->numCommands]->numArgumentos++;
	}
}


// ------------------------------ FIN ASIGNACION DE VARIABLES DE ENTORNO ---------------------------------------------------------------------------


// ----------------------- FUNCIONES PARA TOKENIZAR ENTRADA Y SETEAR COMANDOS Y SUS RESPECTIVOS ARGUMENTOS ----------------------------------------------------------------------------------

void
tokenizator(char *line, Commands *cmds) 
{
    
	char *token;
	char *saveptr;


	//SETEAMOS EL PRIMER COMANDO
	if (reserve_commands(cmds) == NULL) {
		// Manejar el error de asignación de memoria
		memLocateFailed();

		return;
	}
		
	token = strtok_r(line, " ", &saveptr); //Token es una dir de memoria
    assignCommandName(cmds->comandos[cmds->numCommands], token);
	//token = strtok_r(NULL, " ", &saveptr);
		//cmds->numCommands++;
	
	
	while (token != NULL) {

		if (cmds->comandos[cmds->numCommands]->numArgumentos != 0) {
			token = strtok_r(NULL, " ", &saveptr);
		}

		if (token != NULL) {
 			//LOGICA PARA DETECTAR PIPES
            if (strcmp(token, "|") == 0) {
                setLastArgumentNull(cmds->comandos[cmds->numCommands]);
                cmds->numCommands++;

                if (reserve_commands(cmds) == NULL) {
                    // Manejar el error de asignación de memoria
                    memLocateFailed();
                    return;
                }

                token = strtok_r(NULL, " ", &saveptr);
                assignCommandName(cmds->comandos[cmds->numCommands], token);
            } else if (strcmp(token, ">") == 0) {
                // Redirección de salida
                char *file_name = strtok_r(NULL, " ", &saveptr);
                cmds->comandos[cmds->numCommands]->salida = strdup(file_name);
                //token = strtok_r(NULL, " ", &saveptr);
            } else if (strcmp(token, "<") == 0) {
                // Redirección de entrada
                char *file_name = strtok_r(NULL, " ", &saveptr);
                cmds->comandos[0]->entrada = strdup(file_name);
                //token = strtok_r(NULL, " ", &saveptr);
			} else if (strchr(token, '$') != NULL) {
				variable_sustitution(cmds, token);

			} else if (strcmp(token, "&") == 0) {
				cmds->background = 1;
            } else if (strchr(token, '=') != NULL) {
				printf("parece que hay una sust \n");
				variable_asig(cmds, token);

				
			}
			else {
                if (reserve_args(cmds->comandos[cmds->numCommands]) == NULL) {
                    // Manejar el error de asignación de memoria
                    memLocateFailed();
                    return;
                }

                cmds->comandos[cmds->numCommands]->argumentos[cmds->comandos[cmds->numCommands]->numArgumentos] = strdup(token);
                cmds->comandos[cmds->numCommands]->numArgumentos++;
                //token = strtok_r(NULL, " ", &saveptr);
            }
        }
    }
    setLastArgumentNull(cmds->comandos[cmds->numCommands]);
    cmds->numCommands++;
}


// ----------------------- FUNCIONES DEDICADAS A LA LECTURA DE LA ENTRADA ---------------------------------------------------------------------

char *
clean_line(char *line)
{
	for (int i = 0; (line[i] != '\0'); i++) {
		if (line[i] == '\n') {
			line[i] = '\0';
		}
	}
	return line;
}

void
read_lines(Commands *cmds)
{

	char *line = (char *)malloc(LINE_BUFFER_SIZE);	// Hacemos una asignación inicial de memoria de 256 caracteres por linea

	malloc_check(line);

	printf("background %d\n", cmds->background);

	//if (cmds->background == 0) {

	fgets(line, LINE_BUFFER_SIZE, stdin);
	

	line = clean_line(line);
	tokenizator(line, cmds);
	
	

	/*}
	else {
		while (fgets(line, LINE_BUFFER_SIZE, stdin) != NULL) {	//Cada vez que llamemos a fgets, se sobreescribirá line

			// EJECUTAMOS EL COMANDO SI RETORNA ERROR ES QUE EL ARCHIVO NO EXISTE POR LO QUE NO HABRA QUE CREAR UNO NUEVO
			line = clean_line(line);	//Limpiamos line porque alfinal tiene un '\n'
				//printf("The line: %s \n", line);
			tokenizator(line, cmds);
				//create_files(line, status);

		}

		if (!feof(stdin)) {
		// Llegamos al final de la entrada estándar
		errx(EXIT_FAILURE, "eof not reached");
		}
	}
	*/
	



	free(line);

}
// ----------------------- FIN DE FUNCIONES DEDICADAS A LA LECTURA DE LA ENTRADA ---------------------------------------------------------------------

// ----------------------- FUNCIONES DEDICADAS A LA BUSCADA DE EJECUTABLES EN DIFERENTES PATHS -------------------------------------------------------

void
searchin_paths(Command *cmd) 
{
	//printf("-----> Buscamos en $PATHS \n");
	
	char *sh_paths = getenv("PATH"); //ESTO NOS DEVUELVE LA LISTA DE LA VAR. PATHS DE LA SHELL, SEPARADOS POR ":", POR LO QUE HAY QUE TOKENIZARLA
		
	char *sh_paths_copy = strdup(sh_paths);

	char *token;
	char *saveptr;
	char *current_dir;

	token = strtok_r(sh_paths_copy, ":", &saveptr);

	while (token != NULL) {
		current_dir = malloc(strlen(token) + 2 + strlen(cmd->nombre));  // +2 para el '/' y '\0' y luego lo que ocupe el nombre del comando
		if (current_dir == NULL) {
			memLocateFailed();
		}
		strcpy(current_dir, token);
		strcat(current_dir, "/");
		strcat(current_dir, cmd->nombre);
		//printf("Se esta buscando en: %s \n", current_dir);

		//BUSCAMOS EL EXEC, Y NOS QUEDAREMOS CON EL ULTIMO PATH DONDE SE HALLA ENCONTRADO SI ES QUE SE HA ENCONTRADO EN VARIOS
		
		if (access(current_dir, F_OK) == 0) {
			if (cmd->path != NULL) {
                free(cmd->path);
            }
			cmd->path = strdup(current_dir);
			//printf("Se ha encontrado en: %s \n", cmd->path);
		}
		free(current_dir);		
		token = strtok_r(NULL, ":", &saveptr);
	}
	free(sh_paths_copy);
}



void
searchin_pwd(Command *cmd) 
{
	
	//printf("-----> Buscamos %s en el working directory \n", cmd->nombre);	
	if (access(cmd->nombre, F_OK) == 0) {
		//printf("El comando %s se ha encontrado en working directory \n", cmd->nombre);
		char *actual_dir = strcat(getcwd(NULL, 0), "/"); //DE ESTA MANERA GETCWD, UTILIZA UN BUFFER DE MEMORIA DINAMICO PARA LA RUTA DEL PATH PWD
		//char *full_path = strcat(actual_dir,"/");
		char *full_path = strcat(actual_dir,cmd->nombre);

		cmd->path = strdup(full_path); //HACEMOS UNA COPIA Y ASIGNAMOS (AHORA YA PODEMOS LIBERAR EL ORIGINAL)
		//create_path(); // CREA UN PATH CON LA RUTA, Y AL FINAL EL NOMBRE DEL ARCHIVO CONCATENADO
		free(actual_dir); //LIBERAMOS EL ORIGINAL
		free(full_path);
	}
	else {
		//printf("El comando %s NO se ha encontrado en working directory \n", cmd->nombre);
		// No lo hemos encontrado en el dir actual, asi que lo buscamos en la lista de paths
		searchin_paths(cmd);			
	}

}

int 
is_builtin(Command *cmd) { // Si es un built-in devuelve la 1
	int found = 0;

	for (int i = 0; builtin_cmds[i] != NULL; i++) {
		if(strcmp(cmd->nombre, builtin_cmds[i]) == 0) {
			found = 1; //ES UN COMANDO BUILT IN
			cmd->path = strdup("built-in");
		}  
	}
	return found; //ES BUILT IN
}


void
search_paths(Commands *cmds) {

	for (int numCommand = 0; numCommand < cmds->numCommands; numCommand++) {
		printf("-------- BUSCANDO EJECUTABLES DE %s -----------\n", cmds->comandos[numCommand]->nombre);
		//searchin_builtins();
		if (is_builtin(cmds->comandos[numCommand]) != 1) {
			searchin_pwd(cmds->comandos[numCommand]); //SI NO ES BUILT-IN TENDREMOS QUE BUSCAR EL EXEC
		} 
		
		// COMPROBAMOS SI SE HA ENCONRADO EN ALGUNA DE LAS FUNCIONES
		if (cmds->comandos[numCommand]->path == NULL) {
			printf("Command %s not found. \n", cmds->comandos[numCommand]->nombre);
			exit(1);
		}
	}

}


// ----------------------- FIN DE FUNCIONES DEDICADAS A LA BUSCADA DE EJECUTABLES EN DIFERENTES PATHS -------------------------------------------------

// ----------------------- LOGICA PARA REDIRECCIONES --------------------------------------------------------------------------------------

void
fd_setter(Command *cmd, int* fd_in, int* fd_out) // ESTA FUNCION REALIZA LAS REDIRECCIONES EN CASO DE QUE HALLA
{

	if (cmd->entrada != NULL) {
		printf("Se ha cambiado la entrada \n");
		*fd_in= open(cmd->entrada, O_RDONLY);
		dup2(*fd_in, STDIN_FILENO);
	} else {
		*fd_in = STDIN_FILENO;
	}
				
	if (cmd->salida != NULL) {
		printf("Se ha cambiado la salida \n");
		*fd_out= open(cmd->salida, O_CREAT | O_WRONLY | O_TRUNC, 0666);
		dup2(*fd_out, STDOUT_FILENO);
	} else {
		*fd_out = STDOUT_FILENO;
	}

}



// ------------------------ FIN DE LOGICA PARA REDIRECCIONES -------------------------------------

// ------------------------- COMANDOS BUILT-INS -------------------------------------------------------------------------------------------


void 
exec_cd(Command *cmd) {
	if (cmd->numArgumentos > 1) {
			// SI HAY MAS DE UN ARGUMENTO SIGNIFICA QUE HAY UN PATH, args: (cd, path)		
			chdir(cmd->argumentos[1]);
	} else {
		// NO NOS HAN DADO UN PATH ASI QUE TENENOS QUE IR A HOME
		char *sh_home = getenv("HOME");
		chdir(sh_home);
	}
}

void 
exec_asig(Command *cmd) {
	printf("Queremos darle a %s el valor %s \n", cmd->argumentos[0], cmd->argumentos[1]);

	if (setenv(cmd->argumentos[0], cmd->argumentos[1],1) != 0) { //EL 1 ES APRA SOBREESCRIBIR LA VARIABLE EN CASO DE QUE YA EXISTA
        err(EXIT_FAILURE, "Error to establish environment variable");
    }

	// Acceder al valor de la variable de entorno
    char *env_value = getenv(cmd->argumentos[0]);
    if (env_value != NULL) {
        printf("Valor de MY_VARIABLE: %s\n", env_value);
    } else {
        printf("MY_VARIABLE no está definida\n");
    }

}


// --------------------------- FIN DE COMANDOS BUILT-INS -----------------------------------------------------------------------------------


// ----------------------- LOGICA DE EJECUCIÓN DE COMANDOS ----------------------------------------------------------------------------------


void
execute_pipe(Commands *cmds)
{
	
	int child;

	int** pipes = malloc((cmds->numCommands - 1)*sizeof(int*));

	for (int pipe = 0; pipe < cmds->numCommands-1; pipe++) {
		pipes[pipe] = malloc(2*sizeof(int));
	}

	for (int numCommand = 0; numCommand < cmds->numCommands; numCommand++) {

		//CREAMOS LOS PIPES
		if (numCommand < cmds->numCommands - 1) {
			if (pipe(pipes[numCommand]) == -1) {
				err(EXIT_FAILURE,
				    "Theres an error creating the first pipe.");
			}
		}

		// LOGICA DE PIPES

		switch (child = fork()) {	// ---------- HIJO  ----------------
		case -1:
			err(EXIT_FAILURE,
			    "Theres an error with the child proccess");
		case 0:

			// redigirigmos la salida
			// DEFINIMOS LA ENTRADA Y SALIDA ESTANDAR
			int fd_in, fd_out;
			fd_setter(cmds->comandos[numCommand], &fd_in, &fd_out);
			//printf("Soy el comando %d y estoy leyendo de %d \n", numCommand, fd_in);
			//printf("Soy el comando %d y mi salida es %d \n", numCommand, fd_out);

			if (numCommand > 0) {	//SI NO ES EL PRIMER COMANDO, LA ENTRADA LA TIENE QUE LEER DEL COMANDO ANTERIOR
				dup2(pipes[numCommand - 1][0], fd_in);	//Si no es el pirmer comando, deberá leer la entrada de la salida del anterior.
				close(pipes[numCommand - 1][0]);	//Como en dup2 ya se ha hecho el duplicado, podemos cerrar el pipe
				close(pipes[numCommand - 1][1]);	//Del pipe que lo precede, solo queremos leer la entrada, por lo tanto cerramos la escritura
			}
			if (numCommand < cmds->numCommands - 1) {	//SI NO ES EL ULTIMO COMANDO, LA SALIDA DEBE SER ENVIADA AL SIGUIENTE COMANDO
				close(pipes[numCommand][0]);	// COMO QUEREMOS ESCRIBIR EN EL PIPE QUE LO UNE CON EL SIG COMANDO, PODEMOS CERRAR EL EXTREMO DE LECTURA
				dup2(pipes[numCommand][1], fd_out);	//REDIRIGIMOS LA SALIDA AL PIPE QUE LO UNE CON EL SIG COMANDO
				close(pipes[numCommand][1]);	// COMO YA HEMOS HECHO EL DUPLICADO CON DUP2, LO PODEMOS BORRAR
			}
			// EJECUTAMOS EL COMANDO Y LO MANEJAMOS EN CASO DE ERROR
			execv(cmds->comandos[numCommand]->path,
			      cmds->comandos[numCommand]->argumentos);
			printf("There's an error executing the comand %s. \n",
			       cmds->comandos[numCommand]->nombre);
			exit(EXIT_FAILURE);
		default:
			// UNA VEZ QUE SE EJECUTE EL COMANDO, EL PADRE CERRARÁ LO QUE YA NO SE VA A VOLVER A USAR
			if (numCommand < cmds->numCommands - 1) {
				close(pipes[numCommand][1]);	// Cierra el extremo de escritura del pipe actual
			}

			
		}
	}


	if (cmds->background == 0) {
		// Código del padre que espera a todos los hijos
		for (int i = 0; i < cmds->numCommands; i++) {
			int status;
			wait(&status);	
			printf("Pipe %d executed\n", i);
		}
	}

	printf("Pipe executing in background\n");
}



void
exec_cmd(Command *cmd, int background)
{
	int child;

	switch (child = fork()) {
	case -1:
		err(EXIT_FAILURE, "Theres an error with the child");
	case 0:
		// SETEAMOS LOS DESCRIPTORES DE FICHERO DE LA ENTRADA Y SALIDA
		int fd_in, fd_out;
		fd_setter(cmd, &fd_in, &fd_out);
		//printf("estoy leyendo de %d \n", fd_in);
		execv(cmd->path, cmd->argumentos);
		
		exit(0);
	default:
		if (background == 0) {
			int status;
			wait(&status);
			printf("Command executed\n");
		}

		printf("Command executing in background\n");
		
	}
}

void
exec_builtin(Command *cmd) {
// COMPROBAMOS CON QUE BUILT-IN SE CORRESPONDE 
	if (strcmp(cmd->nombre, "cd") == 0) { 
		//COMPROBAMOS SI HAY ALGUN PATH COMO ARGUMENTO
		exec_cd(cmd);
	}
	if (strcmp(cmd->nombre, "=") == 0) { 
		//COMPROBAMOS SI HAY ALGUN PATH COMO ARGUMENTO
		exec_asig(cmd);
	}
}


void
exec_cmds(Commands *cmds) 
{
	if (cmds->numCommands > 1) {
		// HAY UN PIPE
		execute_pipe(cmds);
	} 
	else {
		// COMPROBAMOS SI ES UN COMANDO BUILT_IN
		if (is_builtin(cmds->comandos[0]) == 1) {
			exec_builtin(cmds->comandos[0]);
		} else {
			exec_cmd(cmds->comandos[0], cmds->background);
		}
		
	}

}


// ----------------------- FIN LOGICA DE EJECUCIÓN DE COMANDOS ----------------------------------------------------------------------------------


// ----------------------- FUNCIONES DEDICADAS A LA LIBERACIÓN DE MEMORIA ASIGNADA DINAMICAMENTE ------------------------------------------------------
void
free_command(Commands *cmds)
{
	for (int numCommand = 0; numCommand < cmds->numCommands; numCommand++) {
		free(cmds->comandos[numCommand]->nombre);
		free(cmds->comandos[numCommand]->path);

		if (cmds->comandos[numCommand]->salida != NULL) {
			free(cmds->comandos[numCommand]->salida);
		}

		if (cmds->comandos[numCommand]->entrada != NULL) {
			free(cmds->comandos[numCommand]->entrada);
		}
		for (int i = 0; i < cmds->comandos[numCommand]->numArgumentos; i++) {
			free(cmds->comandos[numCommand]->argumentos[i]);
		}
		free(cmds->comandos[numCommand]->argumentos);
		free(cmds->comandos[numCommand]);
	}
}

void
free_commands(Commands *cmds)
{

	free(cmds->comandos);

}

void
free_mem(Commands *cmds)
{
	free_command(cmds);
	free_commands(cmds);
}

// ----------------------- FIN DE FUNCIONES DEDICADAS A LA LIBERACIÓN DE MEMORIA ASIGNADA DINAMICAMENTE ------------------------------------------------------

void 
commands_printer(Commands *cmds) {
	printf("Contador de comandos: %d \n", cmds->numCommands);
	for (int numCommand=0; numCommand <cmds->numCommands; numCommand++) {
		printf("----COMANDO %d ------\n", numCommand);
		printf("Comando: %s \n",cmds->comandos[numCommand]->nombre);
		printf("Path: %s \n",cmds->comandos[numCommand]->path);
		printf("Entrada: %s \n",cmds->comandos[numCommand]->entrada);
		printf("Salida: %s \n",cmds->comandos[numCommand]->salida);
		for(int numArg=0; numArg < cmds->comandos[numCommand]->numArgumentos; numArg++) {
			printf("Argumento %d: %s \n",numArg,cmds->comandos[numCommand]->argumentos[numArg]);
		}
	}

}


int
main(int argc, char *argv[])
{
	int status = 0;
    Commands Comandos;
    

    if (argc > 1) {
        err(EXIT_FAILURE, "No arguments needed.");
    }

	
	do {
		if (Comandos.background == 1) {
			printf("Limpiando comandos \n");
			free_command(&Comandos);
		}
		
		initializerCommands(argv, &Comandos);
		//Comandos.background = 0;
		read_lines(&Comandos);
        search_paths(&Comandos);
        commands_printer(&Comandos);
        printf("------------------------------- EJECUCION en plano: %d --------------------------- \n", Comandos.background);
        exec_cmds(&Comandos);
	} while (1);	
	//} while (Comandos.background == 1); //Cuando no se accede al campo a traves de un puntero se pone "." en vez de "->"
        //free_command(&Comandos);
        //free_command(&Comandos);
	printf("\n");

    free_commands(&Comandos);
    exit(status);
    return 0;

}