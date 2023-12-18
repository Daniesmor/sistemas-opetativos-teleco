#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <sys/wait.h>
#include <string.h>

// ----------------------- ESTRUCTURAS DE DATOS UTILIZADAS --------------------------------------------------------------------------------------

enum {
	MAX_PATH_LENGTH = 256,
	MAX_FILENAME_LENGTH = 100,
	LINE_BUFFER_SIZE = MAX_PATH_LENGTH * sizeof(char),
	FILENAME_BUFFER_SIZE = MAX_FILENAME_LENGTH * sizeof(char),
	READ_PERMISSION = 0444,
	RW_PERMISSION = 0640,
    NUM_PARAMS = 3,
};

struct Command {
	char *nombre;	// Campo para el nombre del comando
	char *path;
	int numArgumentos;	// Contador para llevar el seguimiento de la cantidad de argumentos
	char **argumentos;
};

typedef struct Command Command;


struct Commands {

    int numCommands;
    Command **comandos;

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


// ----------------------------- FIN DE GESTION DEL STRUCT COMMANDS -----------------------------------------------------------------------------



// ----------------------- FUNCIONES PARA TOKENIZAR ENTRADA ----------------------------------------------------------------------------------

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
		
		//cmds->numCommands++;
		
	while (token != NULL) {

		if (cmds->comandos[cmds->numCommands]->numArgumentos != 0) {
			token = strtok_r(NULL, " ", &saveptr);
		}

		if (token != NULL) {
			if (strcmp(token, "|") == 0) { //strcmp() compara el contenido de las cadenas token y "|". Si son iguales, devuelve cero; de lo contrario, devuelve un valor distinto de cero.
				cmds->numCommands++;

				if (reserve_commands(cmds) == NULL) {
					// Manejar el error de asignación de memoria
					memLocateFailed();
					return;
				}

				token = strtok_r(NULL, " ", &saveptr);
				assignCommandName(cmds->comandos[cmds->numCommands], token);
			}

			if (reserve_args(cmds->comandos[cmds->numCommands]) == NULL) {
				// Manejar el error de asignación de memoria
				memLocateFailed();
				return;
			}
				
			cmds->comandos[cmds->numCommands]->argumentos[cmds->comandos[cmds->numCommands]->numArgumentos] = strdup(token);
			cmds->comandos[cmds->numCommands]->numArgumentos++;

		}
	}

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
read_lines(int *status, Commands *cmds)
{

	char *line = (char *)malloc(LINE_BUFFER_SIZE);	// Hacemos una asignación inicial de memoria de 256 caracteres por linea

	malloc_check(line);

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

	free(line);

}
// ----------------------- FIN DE FUNCIONES DEDICADAS A LA LECTURA DE LA ENTRADA ---------------------------------------------------------------------

void
free_command(Commands *cmds)
{
	for (int numCommand = 0; numCommand < cmds->numCommands; numCommand++) {
		free(cmds->comandos[numCommand]->nombre);
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
	//free(cmds);
}

void
free_mem(Commands *cmds)
{
	free_command(cmds);
	free_commands(cmds);
}

void 
commands_printer(Commands *cmds) {

	for (int numCommand=0; numCommand <cmds->numCommands; numCommand++) {
		printf("----COMANDO %d ------\n", numCommand);
		printf("%s \n",cmds->comandos[numCommand]->nombre);
		for(int numArg=0; numArg < cmds->comandos[numCommand]->numArgumentos; numArg++) {
			printf("Argumento %d: %s \n",numArg,cmds->comandos[numCommand]->argumentos[numArg]);
		}
	}

}




int
main(int argc, char *argv[])
{
	int status = 0;
    //Command *Commands[NUM_PARAMS];
	Commands Comandos;
    initializerCommands(argv, &Comandos);

	if (argc > 1) {
		err(EXIT_FAILURE, "No arguments needed.");
	}

	read_lines(&status, &Comandos);
	commands_printer(&Comandos);
	free_mem(&Comandos);
	
	exit(status);
	return 0;

}
