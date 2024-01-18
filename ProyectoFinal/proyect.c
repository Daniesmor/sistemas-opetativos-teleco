#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <glob.h>

/*

gcc -Wall -Wshadow -Wvla -g -c proyect.c
gcc -g -o proyect proyect.o
valgrind --leak-check=yes ./proyect

*/

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
	char *nombre;		// Campo para el nombre del comando
	char *path;
	int numArgumentos;	// Contador para llevar el seguimiento de la cantidad de argumentos
	char **argumentos;
	char *entrada;
	char *salida;
	char *here;
};

typedef struct Command Command;

struct Commands {

	int numCommands;
	Command **comandos;
	int background;

};

typedef struct Commands Commands;

//Commands Comandos;
// ----------------------- FIN DE ESTRUCTURAS DE DATOS UTILIZADAS --------------------------------------------------------------------------------------

// ------------------------ ALGUNAS ACCIONES BÁSICAS -----------------------------------------------------------------------------------------

char *
clean_line(char *line, char character)
{
	for (int i = 0; (line[i] != '\0'); i++) {
		if (line[i] == character) {
			line[i] = '\0';
		}
	}
	return line;
}

// ------------------------- FIN DE ALGUNAS ACCIONES BASICAS ---------------------------------------------------------------------------------

// -------------------- GESTIÓN DE ERRORES DEL PROGRAMA -----------------------------------------------------------------------------------------

void
memLocateFailed()
{
	err(EXIT_FAILURE,
	    "Error: Could not allocate memory for the argument.\n");
}

int
malloc_check(char *line)
{

	if (line == NULL) {
		memLocateFailed();	// Mensaje de error con información adicional
	}
	return 0;
}

int
cmd_malloc_check(Command *cmd)
{

	if (cmd == NULL) {
		memLocateFailed();	// Mensaje de error con información adicional
	}
	return 0;
}

void
write_failed()
{
	err(EXIT_FAILURE, "Error: Write failed. \n");
}

void
pipe_malloc_check(int *pipe)
{

	if (pipe == NULL) {
		memLocateFailed();	// Mensaje de error con información adicional
	}

}

int
open_check(int fd)
{
	int opened = 0;

	if (fd < 0) {
		printf("The file doesn't exist. \n");
		opened = 1;
	}

	return opened;
}

// ---------------------- FIN DE GESTIÓN DE ERRORES DEL PROGRAMA------------------------------------------------------------------------------

// ------------------------ INICIALIZACION DE VAR DE ENTORNO result -------------------------------------------------------------------

void
initialize_result()
{
	setenv("result", "0", 1);
}

// --------------------------------------------------------------------------------------------------------------------------------------------

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
	cmd->here = NULL;

}

char **
reserve_args(Command *cmd)
{
	// Reserva espacio para tener una lista con tantos punteros como argumentos halla.

	cmd->argumentos =
	    realloc(cmd->argumentos, (cmd->numArgumentos + 1) * sizeof(char *));
	if (cmd->argumentos == NULL) {
		memLocateFailed();
	}
	return cmd->argumentos;

}

void
check_empty_name(Command *cmd)
{
	if (cmd->nombre != NULL) {	//ESTA COMPROBACION ES UTIL PARA LA LOGICA DE IFOK E IFNOT
		free(cmd->nombre);
	}
	cmd->nombre = NULL;
}

void
check_empty_path(Command *cmd)
{
	if (cmd->path != NULL) {	//ESTA COMPROBACION ES UTIL PARA LA LOGICA DE IFOK E IFNOT
		free(cmd->path);
	}
	cmd->path = NULL;
}

void
assignCommandName(Command *cmd, char *name)
{
	check_empty_name(cmd);
	cmd->nombre = strdup(name);
	malloc_check(cmd->nombre);
}

void
assignCommandPath(Command *cmd, char *path)
{
	check_empty_path(cmd);
	cmd->path = strdup(path);
	malloc_check(cmd->path);
}

void
assignCommandArg(Command *cmd, char *arg)
{
	cmd->argumentos[cmd->numArgumentos] = strdup(arg);
	malloc_check(cmd->argumentos[cmd->numArgumentos]);
}

// ----------------------------- FIN DE GESTION DEL STRUCT COMMAND ----------------------------------------------------------------------

// ---------------------------- GESTION DEL STRUCT COMMANDS ------------------------------------------------------------------------------------

void
initializerCommands(Commands *cmds)
{

	cmds->numCommands = 0;
	cmds->comandos = NULL;
	cmds->background = 0;

}

Command **
reserve_commands(Commands *cmds)	//GESTIONA LA LISTA QUE CONTENDRÁ LOS PUNTEROS A LOS DIFERENTES COMANDOS
{

	cmds->comandos = realloc(cmds->comandos, (cmds->numCommands + 1) * sizeof(Command *));	//Reserva memoria para una lista de numCOmmands+1 punteros a Commands
	if (cmds->comandos == NULL) {
		memLocateFailed();
	}
	//cmds->comandos[cmds->numCommands] = malloc(sizeof(Command)); // Reservamos memoria para el comando en cuestion
	cmds->comandos[cmds->numCommands] = malloc(sizeof(Command));	// Reservamos memoria para el comando en cuestion
	cmd_malloc_check(cmds->comandos[cmds->numCommands]);

	initializerCommand(cmds->comandos[cmds->numCommands]);	//Inicializamos el comando con los valores predetermiandos

	// RESERVAMOS LA MEMORIA DEL COMANDO EN CUESTION

	return cmds->comandos;	//Retorna la dir de la lista que contiene las direcciones a los comandos

}

void
setLastArgumentNull(Command *cmd)
{
	malloc_check(*reserve_args(cmd));

	cmd->argumentos[cmd->numArgumentos] = NULL;	// Establecer el último elemento como NULL
}

// ----------------------------- FIN DE GESTION DEL STRUCT COMMANDS -----------------------------------------------------------------------------
// ----------------------- FUNCIONES DEDICADAS A LA LIBERACIÓN DE MEMORIA ASIGNADA DINAMICAMENTE DE LOS COMANDOS ------------------------------------------------------

void
free_command(Command *cmd)
{
	free(cmd->nombre);
	free(cmd->path);
	free(cmd->salida);
	free(cmd->entrada);
	free(cmd->here);

	for (int i = 0; i < cmd->numArgumentos; i++) {

		free(cmd->argumentos[i]);

	}
	free(cmd->argumentos);
	free(cmd);
}

void
free_commands(Commands *cmds)
{
	for (int numCommand = 0; numCommand < cmds->numCommands; numCommand++) {
		free_command(cmds->comandos[numCommand]);
	}
	free(cmds->comandos);
}

// ----------------------- FIN DE FUNCIONES DEDICADAS A LA LIBERACIÓN DE MEMORIA ASIGNADA DINAMICAMENTE ------------------------------------------------------

// ------------------------------ ASIGNACION Y SUSTITUCION DE VARIABLES DE ENTORNO (COMANDO ESPECIAL) ---------------------------------------------------------------------------

void
add_asignation_arg(char *vars, Commands *cmds)
{

	if (reserve_args(cmds->comandos[cmds->numCommands]) == NULL) {
		// Manejar el error de asignación de memoria
		memLocateFailed();
	}
	// GUARDAMOS EL VALOR DE LA VARIABLE QUE QUEREMOS CREAR COMO ARGUMENTO 0
	cmds->comandos[cmds->numCommands]->
	    argumentos[cmds->comandos[cmds->numCommands]->numArgumentos] =
	    strdup(vars);
	malloc_check(cmds->comandos[cmds->numCommands]->
		     argumentos[cmds->comandos
				[cmds->numCommands]->numArgumentos]);
	cmds->comandos[cmds->numCommands]->numArgumentos++;

}

void
variable_asig(Commands *cmds, char *token)	// VAMOS A INTERPRETAR ESTE COMANDO COMO UN BUILT-IN PARA SIMPLIFICAR SU PROGRAMACION
{
	char *vars;
	char *saveptr;

	// ASIGNAMOS EL NOMBRE DEL COMANDO COMO "=", PARA PODER COTEJARLO CON LOS BUILTS-IN (lista)
	assignCommandName(cmds->comandos[cmds->numCommands], "=");
	vars = strtok_r(token, "=", &saveptr);	//Token es una dir de memoria
	add_asignation_arg(vars, cmds);

	vars = strtok_r(NULL, " ", &saveptr);	//vars es una dir de memoria
	if (vars != NULL) {
		add_asignation_arg(vars, cmds);
	}

}

void
sust_cmd(Commands *cmds, char *token)	// VAMOS A INTERPRETAR ESTE COMANDO COMO UN BUILT-IN PARA SIMPLIFICAR SU PROGRAMACION
{				//CODIGO UTILIZADO EN CASO DE QUE SOLO SE ESCRIBA $VAR, SIN COMANDO PREVIO
	char *vars;
	char *saveptro;

	vars = strtok_r(token, "$", &saveptro);
	assignCommandName(cmds->comandos[cmds->numCommands], "$");

	if (reserve_args(cmds->comandos[cmds->numCommands]) == NULL) {
		// Manejar el error de asignación de memoria
		memLocateFailed();
		return;
	}

	cmds->comandos[cmds->numCommands]->
	    argumentos[cmds->comandos[cmds->numCommands]->numArgumentos] =
	    strdup(vars);
	malloc_check(cmds->comandos[cmds->numCommands]->
		     argumentos[cmds->comandos
				[cmds->numCommands]->numArgumentos]);
	cmds->comandos[cmds->numCommands]->numArgumentos++;

}

char *
get_next_token(char *ptr, char *delimiter)
{
	return strtok(ptr, delimiter);
}

char *
get_variable(char *found)
{
	char *variable;

	variable = getenv(found);
	if (variable == NULL) {
		variable = "";	//SI LA VARIABLE NO EXISTE SE DEJA VACIA (ESTANDARES)
		//printf("%ld\n", sizeof(variable));
	}

	return variable;
}

void
write_newtoken(char **new_token, char *variable, char *original_token_copy)
{

	//*new_token[0] = '\0'; //INICIALIZAMOS EL STRING CON UNA CADENA VACIA PARA ASEGURARSE DE QUE TENGA CARCTER NULO DE TERMINACION

	if (strcmp(variable, "") == 0) {
		strcpy(*new_token, original_token_copy);
	} else {

		strcpy(*new_token, variable);
		//strcat(*new_token, "/");
		strcat(*new_token, original_token_copy);
	}

}

void
remake_token(char **new_token, char *variable, char *original_token_copy)
{
	//printf("valor de ptr %s \n", ptr--);

	// AHORA DEBEMOS RECONSTRUIR EL TOKEN, CON LA VARIABLE DELIMITADA

	*new_token = malloc(strlen(variable) + strlen(original_token_copy) + 2);	//El 1 es del '/0' y otro por el "/"

	if (*new_token != NULL) {
		write_newtoken(*&new_token, variable, original_token_copy);

	} else {
		memLocateFailed();
	}

}

void
sust_vars(char **token, char **new_token)
{				//SE ENCARGA DE SUSTITUIR UNA VARIABLE DE ENTORNO EN EL TOKEN, Y DEJARLO INTACTO

	char *ptr;
	char *found = NULL;
	char *variable;
	char *delimiter = " /\t";
	char *original_token_copy;

	//char *new_token;

	ptr = strchr(*token, '$');
	ptr++;			//AVANCAMOS UNA POSICION, ES DECIR AL SIGUIENTE CARACTER DESPUES DE $
	//printf("ptr 1: %s \n", ptr);
	original_token_copy = strdup(ptr);
	malloc_check(original_token_copy);
	found = get_next_token(ptr, delimiter);	// BUSCAMOS LA PALABRA DELIMITADA, EN CASO DE QUE HAYA ALGO DESPUES DE LA VAR
	//printf("esto que es: %s \n", found);
	//printf("ptr 2: %s \n", ptr);
	//printf("strelen: %ld \n", strlen(found));
	original_token_copy = original_token_copy + strlen(found);	// ESTO SERÁ EL TEXTO QUE HAY DESPUES DE LA VARIABLE QUE QUEREMOS SUSTITUIR
	//original_token_copy = original_token_copy + (strlen(found)+1);
	//printf("origina: %s \n", original_token_copy);
	if (found != NULL) {

		// OBTENEMOS EL TEXTO DESPUES DE LA PALABRA ENCONTRADA
		//ptr = ptr + (strlen(found)+1);        // NOS SITUAMOS INMEDIATAMENTE DESPUES DEL LA VARIABLE A SUSTITUIR
		//printf("%s \n", ptr);
		variable = get_variable(found);

		if (*original_token_copy == '\0') {
			*new_token = strdup(variable);
			malloc_check(*new_token);
		} else {
			remake_token(&*new_token, variable, original_token_copy);	//RECONSTRUIMOS EL TOKEN, CON LA VARIABLE YA SUSTITUIDA
		}

		/* DOCUMENTACION RAPIDA DE FUNCIONAMIENTO
		   Coge el token, busca cual es la variable que hay que sustituior (found), una vez
		   encontrada se suma al valor de la posicion de memoria de ptr, y comprobamos 
		   si despues de la variable habia mas texto, en cuyo caso se sustituye y se 
		   reescribe el token, en caso contrario se copia la dir de la variable ya encontrada. */

	} else {
		//free(*token);
		*new_token = strdup("$");
		malloc_check(*new_token);
	}

	;
	if (original_token_copy != NULL) {	//L

		free(original_token_copy - strlen(found));	//NOS SITUAMOS EN LA POSICION INICIAL EN LA QUE SE HIZO EL STRDUP PARA LIBERAR TODO
	}

}

// ------------------------------ FIN DE ASIGNACION Y SUSTITUCION DE VARIABLES DE ENTORNO (COMANDO ESPECIAL) ---------------------------------------------------------------------------

// ------------------------------ LOGICA PARA EL OPCIONAL III (Implementacion de globbing) ------------------------------------------------

int
is_glob(char *token)
{
	int found = 0;

	if (strchr(token, '*')) {
		found = 1;
	}
	if (strchr(token, '?')) {
		found = 1;
	}
	if (strchr(token, '[')) {
		found = 1;
	}
	if (strchr(token, ']')) {
		found = 1;
	}
	return found;
}

int
check_glob(char *token, glob_t * glob_result)
{
	//glob_t glob_result; // ES UN STRUCT, QUE CONTIENE LOS CAMPOS gl_pathc y gl_pathv
	int return_value;	// VALOR QUE DEVOLVERÁ LA FUNCIÓN glob()

	//ESTE SERÁ EL PATRÓN DEE BUSQUEDA, QUE CONTENDRA LOS MULTIPLES PATRONES SEPARADAOS POR '|'
	return_value = glob(token, 0, NULL, glob_result);

	return return_value;

}

// ------------------------------ FIN LOGICA PARA EL OPCIONAL III (Implementacion de globbing) ------------------------------------------------

// ------------------------------- LOGICA PARA EL OPCIONAL I (IMPLEMENTACION DE HERE{}) ------------------------------------------------------------

void
create_here_pipes(Command *cmd, int pipe_here[2])
{
	if (cmd->here != NULL) {

		// Crear un pipe
		if (pipe(pipe_here) == -1) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
	}
}

void
close_here_child_pipes(Command *cmd, int pipe_here[2], int input)
{
	if (cmd->here != NULL) {
		close(pipe_here[1]);	// CERRAMOS EL EXTREMO DE ESCRITURA DEL PIPE, YA QUE LEEMOS EL HERE{} DEL PADRE

		// Redirigir la entrada estándar al extremo de lectura del pipe
		if (dup2(pipe_here[0], input) == -1) {	// REDIRIGIMOS LA ENTRADA ESTANDAR AL EXTREMO DE ESCRITURA DEL PIPE
			perror("dup2");
			exit(EXIT_FAILURE);
		}

		close(pipe_here[0]);	// UNA VEZ DUPLICADO CON DUP2, CERRAMOS EL EXTREMO DE LECTURA TB
	}
}

void
close_here_father_pipes(Command *cmd, int pipe_here[2])
{
	if (cmd->here != NULL) {
		close(pipe_here[0]);
		// Escribir la cadena en el extremo de escritura del pipe
		if (write(pipe_here[1], cmd->here, strlen(cmd->here)) == -1) {
			write_failed();
		}		// ESCRIBIMOS POR EL PIPE, LA CADENA QUE CONTIENE DE HERE{}
		close(pipe_here[1]);	// UNA VEZ ESCRITA LA CADENA LA PODEMOS CERRAR
	}
}

void
first_optional(Commands *cmds)
{
	char *line = (char *)malloc(LINE_BUFFER_SIZE);	// Hacemos una asignación inicial de memoria de 256 caracteres por linea

	malloc_check(line);

	int total_size = LINE_BUFFER_SIZE;
	int total_read = 0;	//esto representará lo que hemos leido en total de toda las llamadas a fgets

	//HAY QUE TENER EN CUENTA QUE linea NO ME AÑADE \O, solo \n

	do {
		if (fgets(line + total_read, LINE_BUFFER_SIZE, stdin) == NULL) {
			break;	// ERROR O EOF
		}
		// fgets empezará a escribir en la dir de memoria line + total_read
		// leera como maximo LINE_BUFFER_SIZE
		// lo leera de stdin

		size_t last_read = strlen(line + total_read);	// ESTA VARIABLE NOS DICE, CUANTO TEXTO HEMOS LEIDO EN LA ULTIMA INTERVENCION

		//total_read = total_read + last_read;

		// SI EL ULTIMO CARACTER NO ES \n SIGNIFICA QUE NO HEMOS LEIDO LA LINEA ENTERA
		total_read += last_read;	// ACTUALIZAMOS total_read CON LO QUE HEMOS LEIDO EN LA ULTIMA LLA,ADA
		total_size += LINE_BUFFER_SIZE;	// SUMAMOS OTROS 256 CARACTERES
		line = realloc(line, total_size);	// Hacemos un realloc con el nuevo tamaño, y continuaremos leyendo desde line + current_size
		malloc_check(line);

	}
	while (strchr(line, '}') == NULL);	//MIENTRAS QUE NO ENCONTREMOS EL FIN DE HERE{} SEGUIMOS LEYENDO

	clean_line(line, '}');
	//printf("Linea de here: %s \n", line);
	cmds->comandos[0]->here = strdup(line);
	malloc_check(cmds->comandos[0]->here);
	free(line);
}

// ------------------------------- FIN LOGICA PARA EL OPCIONAL I (IMPLEMENTACION DE HERE{}) ------------------------------------------------------------

// ----------------------- FUNCIONES PARA TOKENIZAR ENTRADA Y SETEAR COMANDOS Y SUS RESPECTIVOS ARGUMENTOS ----------------------------------------------------------------------------------

void
instruction_pipe(Commands *cmds, char **token, char **saveptr)
{
	setLastArgumentNull(cmds->comandos[cmds->numCommands]);
	cmds->numCommands++;

	if (reserve_commands(cmds) == NULL) {
		// Manejar el error de asignación de memoria
		memLocateFailed();
		return;
	}

	*token = strtok_r(NULL, " ", *&saveptr);
	assignCommandName(cmds->comandos[cmds->numCommands], *token);
}

void
instruction_output_redirection(Commands *cmds, char **token, char **saveptr)
{
	char *file_name = strtok_r(NULL, " ", *&saveptr);

	cmds->comandos[cmds->numCommands]->salida = strdup(file_name);
	malloc_check(cmds->comandos[cmds->numCommands]->salida);
	//*token = strtok_r(NULL, " ", *&saveptr);
}

void
instruction_input_redirection(Commands *cmds, char **token, char **saveptr)
{
	char *file_name = strtok_r(NULL, " ", *&saveptr);

	cmds->comandos[0]->entrada = strdup(file_name);
	malloc_check(cmds->comandos[0]->entrada);
	//*token = strtok_r(NULL, " ", *&saveptr);
}

void
instruction_here(Commands *cmds)
{
	if (cmds->background == 0) {	//SI EL COMANDO NO TIENE "&", LEERA LA ENTRADA ESTANDAR DE HERE
		first_optional(cmds);
	}
}

void
instruction_background(Commands *cmds)
{
	cmds->background = 1;
}

void
instruction_regular(Commands *cmds, char **token)
{

	if (cmds->comandos[cmds->numCommands]->nombre != NULL) {
		if (reserve_args(cmds->comandos[cmds->numCommands]) == NULL) {
			// Manejar el error de asignación de memoria
			memLocateFailed();
			return;
		}
		assignCommandArg(cmds->comandos[cmds->numCommands], *token);
		cmds->comandos[cmds->numCommands]->numArgumentos++;
	} else {
		assignCommandName(cmds->comandos[cmds->numCommands], *token);
	}

}

void
instruction_classifier(Commands *cmds, char **token, char **saveptr)
{

	if (strcmp(*token, "|") == 0) {
		instruction_pipe(cmds, token, saveptr);
	} else if (strcmp(*token, ">") == 0) {
		// Redirección de salida
		instruction_output_redirection(cmds, token, saveptr);
	} else if (strcmp(*token, "<") == 0) {
		// Redirección de entrada
		instruction_input_redirection(cmds, token, saveptr);

	} else if (strstr(*token, "HERE{") != NULL) {
		instruction_here(cmds);
	} else if (strcmp(*token, "&") == 0) {
		instruction_background(cmds);

	} else if (strchr(*token, '=') != NULL) {
		variable_asig(cmds, *token);
	} else {
		instruction_regular(cmds, token);

	}

}

int
envar_detector(Commands *cmds, char **token, char *saveptr)
{
	char *new_token;

	if (strchr(*token, '$') != NULL) {

		/*
		   if (cmds->comandos[cmds->numCommands]->nombre == NULL) {
		   sust_cmd(cmds, *token);
		   } */
		sust_vars(*&token, &new_token);
		//printf("EL nuevo token es: %s \n", new_token);
		instruction_classifier(cmds, &new_token, &saveptr);
		/* EL TOKEN YA TIENE LA VARIABLE SUSTITUIDA */
		free(new_token);
		return 1;
	} else {
		return 0;
	}
}

void
globbing_classifier(Commands *cmds, char *token, char *saveptr)
{
	glob_t glob_result;	// ES UN STRUCT, QUE CONTIENE LOS CAMPOS gl_pathc y gl_pathv

	int patterns_found = check_glob(token, &glob_result);

	if (patterns_found == 0) {
		for (int i = 0; i < glob_result.gl_pathc; i++) {
			//printf("Archivo encontrado: %s\n", glob_result.gl_pathv[i]);
			token = glob_result.gl_pathv[i];
			instruction_classifier(cmds, &token, &saveptr);
		}

	}
	globfree(&glob_result);	// LIBEREAMOS MEMORIA UTILIZADA POR glob_result

}

int
glob_detector(Commands *cmds, char **token, char *saveptr)
{
	if (is_glob(*token) != 0) {

		globbing_classifier(cmds, *token, saveptr);
		return 1;
	} else {
		return 0;
	}
}

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

	token = strtok_r(line, " ", &saveptr);	//Token es una dir de memoria

	while (token != NULL) {

		if (cmds->comandos[cmds->numCommands]->numArgumentos != 0) {
			token = strtok_r(NULL, " ", &saveptr);
		}

		if (token != NULL) {

			if ((envar_detector(cmds, &token, saveptr)
			     || glob_detector(cmds, &token, saveptr)) == 0) {
				instruction_classifier(cmds, &token, &saveptr);
			}
		}
	}

	setLastArgumentNull(cmds->comandos[cmds->numCommands]);
	cmds->numCommands++;

}

// ------------------------------ FUNCIONES DEDICADAS A FORMATEAR LA ENTRADA ANTES DE PASARLA AL TOKENIZADOR --------------------------

void
rewrite_line(char *line, char *nuevoString)
{
	int a = 0;
	int b = 0;

	while (line[a] != '\0') {
		if (line[a] == '|' || line[a] == '<' || line[a] == '>') {
			nuevoString[b] = ' ';
			b++;
			nuevoString[b] = line[a];
			b++;
			a++;
			nuevoString[b] = ' ';
			b++;
		} else {
			nuevoString[b] = line[a];
			b++;
			a++;
		}
	}

	nuevoString[b] = '\0';
}

void
remove_tabs(char *line, char *linenotabs)
{
	char *token;
	char *saveptr;

	token = strtok_r(line, " \t", &saveptr);

	strcpy(linenotabs, token);
	strcat(linenotabs, " ");
	while (token != NULL) {

		token = strtok_r(NULL, " \t", &saveptr);

		if (token != NULL) {

			strcat(linenotabs, token);
			strcat(linenotabs, " ");
		}
	}
}

void
formatter(char *line, Commands *cmds)
{

	int longitud = strlen(line);

	char *linenotabs = (char *)malloc((2 * longitud + 1) * sizeof(char));

	malloc_check(linenotabs);

	char *nuevoString = (char *)malloc((2 * longitud + 1) * sizeof(char));

	malloc_check(nuevoString);

	remove_tabs(line, linenotabs);	// ESTA FUNCION QUITA LAS TABULACIONES Y LE DA FORMATO DE ESPACIOS

	//printf("nuevostring: %s \n", linenotabs);

	rewrite_line(linenotabs, nuevoString);	//ESTO DA FORMATO A LAS LINEA EN CASO DE QUE NO HALLA ESPACIOS ENTRE LOS CARACTERES ESPECIALES

	//printf("strlen de line: %d \n", longitud);
	//printf("strlen de nuevostring: %ld \n", strlen(nuevoString));
	//printf("nuevostring: %s \n", nuevoString);

	if (nuevoString != NULL) {
		//printf("Nuevo string: %s \n", nuevoString);
		tokenizator(nuevoString, cmds);
		free(nuevoString);
		free(linenotabs);
	} else {
		memLocateFailed();
	}
}

// ------------------------FIN DE FUNCIONES DEDICADAS A FORMATEAR LA ENTRADA ANTES DE PASARLA AL TOKENIZADOR --------------------------

// ----------------------- FUNCIONES DEDICADAS A LA BUSCADA DE EJECUTABLES EN DIFERENTES PATHS -------------------------------------------------------

void
create_currentdir(char *token, Command *cmd, char **current_dir)
{				//CREA LA RUTA (solo el path), CON EL NOMBRE DE PROGRAMA ETC...

	*current_dir = malloc(strlen(token) + 2 + strlen(cmd->nombre));	// +2 para el '/' y '\0' y luego lo que ocupe el nombre del comando
	malloc_check(*current_dir);

	strcpy(*current_dir, token);
	strcat(*current_dir, "/");
	strcat(*current_dir, cmd->nombre);

}

void
access_dir(char *current_dir, Command *cmd)
{				//COMPRUEBA SI EXISTE EL EJECUTABLE, EN TAL CASO LO PONE EN path
	if (access(current_dir, F_OK) == 0) {
		if (cmd->path != NULL) {
			free(cmd->path);
		}
		cmd->path = strdup(current_dir);
		malloc_check(cmd->path);
		//printf("Se ha encontrado en: %s \n", cmd->path);
	}
}

void
searchin_paths(Command *cmd)
{
	//printf("-----> Buscamos en $PATHS \n");

	char *sh_paths = getenv("PATH");	//ESTO NOS DEVUELVE LA LISTA DE LA VAR. PATHS DE LA SHELL, SEPARADOS POR ":", POR LO QUE HAY QUE TOKENIZARLA

	// PATH ES UNA VARIABLE QUE ESTÁ ENTODOS LOS SIST LINUX POR LO QUE NO HACE FALTA COMPROBAR SI EXISTIRÁ

	char *sh_paths_copy = strdup(sh_paths);

	malloc_check(sh_paths_copy);

	char *token;
	char *saveptr;
	char *current_dir;

	token = strtok_r(sh_paths_copy, ":", &saveptr);

	while (token != NULL) {

		create_currentdir(token, cmd, &current_dir);	//CREAMOS EL PATH DONDE VAMOS A BUSCAR

		//BUSCAMOS EL EXEC, Y NOS QUEDAREMOS CON EL ULTIMO PATH DONDE SE HALLA ENCONTRADO SI ES QUE SE HA ENCONTRADO EN VARIOS
		//printf("token fuera glob: %s \n", token);
		access_dir(current_dir, cmd);

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
		char *actual_dir = strcat(getcwd(NULL, 0), "/");	//DE ESTA MANERA GETCWD, UTILIZA UN BUFFER DE MEMORIA DINAMICO PARA LA RUTA DEL PATH PWD

		//char *full_path = strcat(actual_dir,"/");
		char *full_path = strcat(actual_dir, cmd->nombre);

		assignCommandPath(cmd, full_path);
		//cmd->path = strdup(full_path); //HACEMOS UNA COPIA Y ASIGNAMOS (AHORA YA PODEMOS LIBERAR EL ORIGINAL)
		//create_path(); // CREA UN PATH CON LA RUTA, Y AL FINAL EL NOMBRE DEL ARCHIVO CONCATENADO
		free(actual_dir);	//LIBERAMOS EL ORIGINAL
		free(full_path);
	} else {
		//printf("El comando %s NO se ha encontrado en working directory \n", cmd->nombre);
		// No lo hemos encontrado en el dir actual, asi que lo buscamos en la lista de paths
		searchin_paths(cmd);
	}

}

int
is_builtin(Command *cmd)
{				// Si es un built-in devuelve la 1
	int found = 0;

	for (int i = 0; builtin_cmds[i] != NULL; i++) {
		if (strcmp(cmd->nombre, builtin_cmds[i]) == 0) {
			found = 1;	//ES UN COMANDO BUILT IN
			if (cmd->path != NULL) {
				free(cmd->path);	//SI PATH YA TENIA MEMORIA ASIGNADA, LA LIBERAMOS ANTES DE REDEFINIRLA PARA NO CAUSAR UN LEAK
			}

			cmd->path = strdup("built-in");
			malloc_check(cmd->path);

		}
	}
	return found;		//ES BUILT IN
}

void
search_path(Command *cmd)
{
	if (is_builtin(cmd) != 1) {
		searchin_pwd(cmd);	//SI NO ES BUILT-IN TENDREMOS QUE BUSCAR EL EXEC
	}
	// COMPROBAMOS SI SE HA ENCONRADO EN ALGUNA DE LAS FUNCIONES
	if (cmd->path == NULL) {
		printf("Command %s not found. \n", cmd->nombre);
		setenv("result", "1", 1);
	}
}

void
search_paths(Commands *cmds)
{

	for (int numCommand = 0; numCommand < cmds->numCommands; numCommand++) {
		//printf("-------- BUSCANDO EJECUTABLES DE %s -----------\n", cmds->comandos[numCommand]->nombre);
		//searchin_builtins();
		search_path(cmds->comandos[numCommand]);
	}

}

// ----------------------- FIN DE FUNCIONES DEDICADAS A LA BUSCADA DE EJECUTABLES EN DIFERENTES PATHS -------------------------------------------------

// ----------------------- LOGICA PARA REDIRECCIONES --------------------------------------------------------------------------------------

void
fd_setter(Command *cmd, int *fd_in, int *fd_out)	// ESTA FUNCION REALIZA LAS REDIRECCIONES EN CASO DE QUE HALLA
{

	if (cmd->entrada != NULL) {
		//printf("Se ha cambiado la entrada \n");
		*fd_in = open(cmd->entrada, O_RDONLY);
		//printf("valor de fd: %d \n", *fd_in);
		if (open_check(*fd_in) == 0) {
			dup2(*fd_in, STDIN_FILENO);
		} else {
			//printf("hemos llegao\n");
			free(cmd->entrada);
			cmd->entrada = NULL;
			//printf("valor de entrada: %s \n", cmd->entrada);
			//*fd_in = -1;
		}
	} else {
		*fd_in = STDIN_FILENO;
	}

	if (cmd->salida != NULL) {
		//printf("Se ha cambiado la salida \n");
		*fd_out = open(cmd->salida, O_CREAT | O_WRONLY | O_TRUNC, 0666);
		if (open_check(*fd_out) == 0) {
			dup2(*fd_out, STDOUT_FILENO);
		} else {
			//printf("hemos llegao\n");
			free(cmd->salida);
			cmd->salida = NULL;
			//printf("valor de entrada: %s \n", cmd->entrada);
			//*fd_in = -1;
		}

	} else {
		*fd_out = STDOUT_FILENO;
	}

}

// ------------------------ FIN DE LOGICA PARA REDIRECCIONES -------------------------------------

// ------------------------ LOGICA PARA SEÑALES -----------------------------------------------------------------------------------------

void
sigchld_handler()
{
	int status;
	pid_t pid;

	// Espera a que cualquier proceso hijo termine
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		if (WIFEXITED(status)) {
			//printf("Proceso hijo %d terminó con estado de salida: %d\n", pid, WEXITSTATUS(status));
			// Actualizar la variable de entorno result
			char result_value[10];

			snprintf(result_value, sizeof(result_value), "%d",
				 WEXITSTATUS(status));
			setenv("result", result_value, 1);
		}
	}

}

// ----------------------- FIN DE LOGICA PARA SEÑALES -------------------------------------------------------------------------------------

// ------------------------ EJECUCION DE PIPES Y COMANDOS INDIVIDUALES ---------------------------------------------------------------------

void
wait_single_child()
{
	int status;

	wait(&status);
	//printf("Command executed\n");
	// ............ ENV VAR "result" ..................
	char wexit[2];		//Una posicion para 0 o 1 (estatus de finalizacion) y otra para "/o"

	if (WIFEXITED(status)) {
		//printf("Estado de salida del hijo: %d\n", WEXITSTATUS(status));
		// ......... Para env var "result" .....................
		sprintf((char *)wexit, "%d", WEXITSTATUS(status));	// Convierte el entero a cadena
		setenv("result", wexit, 1);
		// .........................................................
	} else {
		err(EXIT_FAILURE, "The child ended unnormally");
	}
	// ............................................
}

void
execute_pipe(Commands *cmds)
{

	int child;

	int **pipes = malloc((cmds->numCommands - 1) * sizeof(int *));

	// COMPROBRAMES QUE FUNCIONA AHORA

	for (int pipe = 0; pipe < cmds->numCommands - 1; pipe++) {
		pipes[pipe] = malloc(2 * sizeof(int));
		pipe_malloc_check(pipes[pipe]);
	}

	for (int numCommand = 0; numCommand < cmds->numCommands; numCommand++) {

		// ....................... EN CASO DE HERE{} .........................................
		int pipe_here[2];	//CREAMOS UN PIPE QUE SE UTILIZARÁ EN CASO DE UTILIZAR EL PROTOCOLO HERE{}

		if (cmds->comandos[numCommand]->here != NULL) {

			// Crear un pipe
			if (pipe(pipe_here) == -1) {
				perror("pipe");
				exit(EXIT_FAILURE);
			}
		}
		// .........................................................................................

		//CREAMOS LOS PIPES
		if (numCommand < cmds->numCommands - 1) {
			if (pipe(pipes[numCommand]) == -1) {
				err(EXIT_FAILURE,
				    "Theres an error creating the first pipe.");
			}
		}
		// .......................... ENV VAR "result" EN CASO DE QUE & ..............................
		signal(SIGCHLD, sigchld_handler);	// Esta función haá que se envíe en una señal cuando un proceso hijo termine
		// De esta forma no necesitamos hacer wait->wexistatus para saber como terminó.
		// .................................................................................................

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
			// ......................... EN CASO DE HERE{} ................................................
			if (cmds->comandos[numCommand]->here != NULL) {
				close(pipe_here[1]);	// CERRAMOS EL EXTREMO DE ESCRITURA DEL PIPE, YA QUE LEEMOS EL HERE{} DEL PADRE

				// Redirigir la entrada estándar al extremo de lectura del pipe
				if (dup2(pipe_here[0], fd_in) == -1) {	// REDIRIGIMOS LA ENTRADA ESTANDAR AL EXTREMO DE ESCRITURA DEL PIPE
					perror("dup2");
					exit(EXIT_FAILURE);
				}

				close(pipe_here[0]);	// UNA VEZ DUPLICADO CON DUP2, CERRAMOS EL EXTREMO DE LECTURA TB
			}
			// .................................................................................................
			// EJECUTAMOS EL COMANDO Y LO MANEJAMOS EN CASO DE ERROR
			execv(cmds->comandos[numCommand]->path,
			      cmds->comandos[numCommand]->argumentos);
			printf("There's an error executing the comand %s. \n",
			       cmds->comandos[numCommand]->nombre);
			exit(EXIT_FAILURE);
		default:

			// ......................... EN CASO DE HERE{} ................................................
			if (cmds->comandos[numCommand]->here != NULL) {
				close(pipe_here[0]);
				// Escribir la cadena en el extremo de escritura del pipe
				write(pipe_here[1], cmds->comandos[numCommand]->here, strlen(cmds->comandos[numCommand]->here));	// ESCRIBIMOS POR EL PIPE, LA CADENA QUE CONTIENE DE HERE{}
				close(pipe_here[1]);	// UNA VEZ ESCRITA LA CADENA LA PODEMOS CERRAR
			}
			// ............................................................................................

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
			//printf("Pipe %d executed\n", i);

			// ........... ENV VAR result (se seteará el ultimo valor)................
			char wexit[2];	//Una posicion para 0 o 1 (estatus de finalizacion) y otra para "/o"

			if (WIFEXITED(status)) {
				//printf("Estado de salida del hijo: %d\n", WEXITSTATUS(status));
				// ......... Para env var "result" .....................
				sprintf((char *)wexit, "%d", WEXITSTATUS(status));	// Convierte el entero a cadena
				setenv("result", wexit, 1);
				// .........................................................
			} else {
				err(EXIT_FAILURE, "The child ended unnormally");
			}
			// ....................................................
		}
	}
	// ------------ LIBEREMOS MEMORIA ASIGNADA A LOS PIPES ----------------------------
	for (int pipe = 0; pipe < cmds->numCommands - 1; pipe++) {
		free(pipes[pipe]);
	}
	free(pipes);
	// ---------------------------------------------------------------------------
	//printf("Pipe executing in background\n");
}

void
exec_cmd(Command *cmd, int background)
{

	if (cmd->path != NULL) {
		int child;

		// ............................ EN CASO DE HERE ...................

		int pipe_here[2];	//CREAMOS UN PIPE QUE SE UTILIZARÁ EN CASO DE UTILIZAR EL PROTOCOLO HERE{}

		create_here_pipes(cmd, pipe_here);	// CREAMOS LOS PIPES

		// ........................................................

		// .......................... ENV VAR "result" EN CASO DE QUE & ..............................

		signal(SIGCHLD, sigchld_handler);	// Esta función haá que se envíe en una señal cuando un proceso hijo termine
		// De esta forma no necesitamos hacer wait->wexistatus para saber como terminó.

		// .................................................................................................

		switch (child = fork()) {
		case -1:
			err(EXIT_FAILURE, "Theres an error with the child");
		case 0:
			// SETEAMOS LOS DESCRIPTORES DE FICHERO DE LA ENTRADA Y SALIDA
			int fd_in, fd_out;

			fd_setter(cmd, &fd_in, &fd_out);
			//printf("estoy leyendo de %d \n", fd_in);
			//printf("estoy saliendo de %d \n", fd_out);

			// ......................... EN CASO DE HERE{} ....................................................

			close_here_child_pipes(cmd, pipe_here, STDIN_FILENO);
			// .................................................................................................

			if (fd_in >= 0 && fd_out >= 0) {
				execv(cmd->path, cmd->argumentos);
			}

			exit(1);
		default:
			// ......................... EN CASO DE HERE{} ................................................
			close_here_father_pipes(cmd, pipe_here);
			// ............................................................................................

			if (background == 0) {
				wait_single_child();
			}
			//printf("Command executing in background\n");

		}
	} else {
		setenv("result", "1", 1);
	}

}

// ------------------------ FIN DE EJECUCION DE PIPES Y COMANDOS INDIVIDUALES ---------------------------------------------------------------------

// ------------------------- COMANDOS BUILT-INS -------------------------------------------------------------------------------------------

void
exec_cd(Command *cmd)
{
	// ......... Para env var "result" .....................
	char wexit[2];		//Una posicion para 0 o 1 (estatus de finalizacion) y otra para "/o"
	int status = 0;

	// ..................................................

	if (cmd->numArgumentos > 1) {
		// SI HAY MAS DE UN ARGUMENTO SIGNIFICA QUE HAY UN PATH, args: (cd, path)               
		if (chdir(cmd->argumentos[1]) != 0) {
			status = 1;
			printf("The dir doesn't exist. \n");
		}
	} else {
		// NO NOS HAN DADO UN PATH ASI QUE TENENOS QUE IR A HOME
		char *sh_home = getenv("HOME");

		chdir(sh_home);
	}

	// ............. Definimos env var "result" .....................
	sprintf((char *)wexit, "%d", status);
	setenv("result", wexit, 1);	// Actualizar el valor de "result" dependiendo del estado
	// ...............................................................
}

void
exec_asig(Command *cmd)
{

	// ......... Para env var "result" .....................
	char wexit[2];		//Una posicion para 0 o 1 (estatus de finalizacion) y otra para "/o"
	int status = 0;

	// ..................................................

	//printf("Queremos darle a %s el valor %s \n", cmd->argumentos[0], cmd->argumentos[1]);

	if (setenv(cmd->argumentos[0], cmd->argumentos[1], 1) != 0) {	//EL 1 ES APRA SOBREESCRIBIR LA VARIABLE EN CASO DE QUE YA EXISTA
		status = 1;
		err(EXIT_FAILURE, "Error to establish environment variable");
	}
	// Acceder al valor de la variable de entorno
	char *env_value = getenv(cmd->argumentos[0]);

	if (env_value == NULL) {
		status = 1;
		printf("%s not defined.\n", cmd->argumentos[0]);
	}
	// ............. Definimos env var "result" .....................
	sprintf((char *)wexit, "%d", status);
	setenv("result", wexit, 1);	// Actualizar el valor de "result" dependiendo del estado
	// ...............................................................

}

void
exec_sust(Command *cmd)
{				// EN CASO DE COMANDO: Ej: $PATH PATH: ARG[0]

	// ......... Para env var "result" .....................
	char wexit[2];		//Una posicion para 0 o 1 (estatus de finalizacion) y otra para "/o"
	int status = 0;

	// ..................................................

	char *variable = getenv(cmd->argumentos[0]);

	if (variable == NULL) {
		status = 1;
		printf("error: var %s does not exist. \n", cmd->argumentos[0]);

	} else {
		printf("%s \n", variable);
	}

	// ............. Definimos env var "result" .....................
	sprintf((char *)wexit, "%d", status);
	setenv("result", wexit, 1);	// Actualizar el valor de "result" dependiendo del estado
	// ...............................................................

}

void
remake_cmd(Command *cmd, int *background)
{				// FUNCION UTILIZADA PARA IFOK E IFNOT
	// Tenemos que rehacer el comando para pasarselo al ejecutador de comandos.

	check_empty_path(cmd);

	assignCommandName(cmd, cmd->argumentos[1]);

	for (int i = 1; i < cmd->numArgumentos; i++) {
		if (strcmp(cmd->argumentos[i], "&") == 0) {
			*background = 1;
		}
		if (cmd->argumentos[i - 1] != NULL) {
			free(cmd->argumentos[i - 1]);
		}
		cmd->argumentos[i - 1] = strdup(cmd->argumentos[i]);
	}

	cmd->numArgumentos = cmd->numArgumentos - 1;
	free(cmd->argumentos[cmd->numArgumentos]);	// LA LIBERACIONSE HACE DESPUES, RECUERDA QUE LOS ARGUMENTOS VAN DE 0 A numArg -1

	setLastArgumentNull(cmd);
}

void
exec_ifok(Command *cmd)
{

	char *prev_status = getenv("result");

	//printf("este es el valor de prev: %s \n", prev_status);

	if (prev_status == NULL) {
		printf("There's no a prev command status. \n");
	} else {
		if (atoi(prev_status) == 0) {	//SIGNIFICA QUE EL COMANDO PREVIO, HA SIDO EXITOSO, EN CASO CONTRARIO NO HACE NADA
			int background = 0;	// en el struct Commnds, gracias al tokenizador ya sabemos el valor, pero necesitamos saberlo para enviarselo a exec_cmd

			remake_cmd(cmd, &background);
			search_path(cmd);
			exec_cmd(cmd, background);
		}
	}

}

void
exec_ifnot(Command *cmd)	//SI IFNOT NO EJECUTA EL COMANDO, SSERA UN STATUS DE ERROR
{

	char *prev_status = getenv("result");

	if (prev_status == NULL) {
		printf("There's no a prev command status. \n");
	} else {
		if (atoi(prev_status) != 0) {	//SIGNIFICA QUE EL COMANDO PREVIO, NO HA SIDO EXITOSO, EN CASO CONTRARIO NO HACE NADA
			// Tenemos que rehacer el comando para pasarselo al ejecutador de comandos.

			int background = 0;	// en el struct Commnds, gracias al tokenizador ya sabemos el valor, pero necesitamos saberlo para enviarselo a exec_cmd

			remake_cmd(cmd, &background);
			search_path(cmd);
			//printf("algo aqui? \n");

			exec_cmd(cmd, background);	//SI PONEMOS ifnot lsa, lsa no existe, por tanto ifnot fallará

		}
		/* ACLARACION: SI EL COMANDO ANTERIOR NO ERA EXISTOSO, ESTE SE EJECUTA, LO QUE HARÁ QUE LA VARIABLE
		   "result" se cambie dentro del exec_cmd */
	}

}

// --------------------------- FIN DE COMANDOS BUILT-INS -----------------------------------------------------------------------------------

// ----------------------- LOGICA DE EJECUCIÓN DE COMANDOS ----------------------------------------------------------------------------------

void
exec_builtin(Command *cmd)
{
// COMPROBAMOS CON QUE BUILT-IN SE CORRESPONDE 
	if (strcmp(cmd->nombre, "cd") == 0) {
		//COMPROBAMOS SI HAY ALGUN PATH COMO ARGUMENTO
		exec_cd(cmd);
	}
	if (strcmp(cmd->nombre, "=") == 0) {
		//COMPROBAMOS SI HAY ALGUN PATH COMO ARGUMENTO
		exec_asig(cmd);
	}
	if (strcmp(cmd->nombre, "$") == 0) {
		exec_sust(cmd);
	}
	if (strcmp(cmd->nombre, "ifok") == 0) {
		exec_ifok(cmd);
	}
	if (strcmp(cmd->nombre, "ifnot") == 0) {
		exec_ifnot(cmd);
	}
}

int
check_all_paths(Commands *cmds)
{				//ESTA FUNCION ES UN NIVEL MAS DE SEGURDIAD PARA EL PIPE
	int path = 0;		// SI PATH == 1 ENTONCES HAY UN CMD QUE NO TIENE PATH ASIGNADO

	for (int numCommand = 0; numCommand < cmds->numCommands; numCommand++) {

		if (cmds->comandos[numCommand]->path == NULL) {
			path = 1;
		}

	}

	return path;
}

void
exec_cmds(Commands *cmds)
{
	if (cmds->numCommands > 1) {
		// HAY UN PIPE
		//COMPROBAMOS SI TODOS LOS CMDS TIENEN UN PATH, ANTES DE EJCUTAR EL PIPE PARA PREVENIR ERRORES
		if (check_all_paths(cmds) == 0) {	//SI check_all_paths == 0 entonces todos los cmds tienen paths.
			execute_pipe(cmds);
		}

	} else {

		// COMPROBAMOS SI ES UN COMANDO BUILT_IN
		if (is_builtin(cmds->comandos[0]) == 1) {
			exec_builtin(cmds->comandos[0]);
		} else {
			exec_cmd(cmds->comandos[0], cmds->background);
		}

	}

}

// ----------------------- FIN LOGICA DE EJECUCIÓN DE COMANDOS ----------------------------------------------------------------------------------

void
commands_printer(Commands *cmds)
{
	printf("Contador de comandos: %d \n", cmds->numCommands);
	for (int numCommand = 0; numCommand < cmds->numCommands; numCommand++) {
		printf("----COMANDO %d ------\n", numCommand);
		printf("Comando: %s \n", cmds->comandos[numCommand]->nombre);
		printf("Path: %s \n", cmds->comandos[numCommand]->path);
		printf("Entrada: %s \n", cmds->comandos[numCommand]->entrada);
		printf("Salida: %s \n", cmds->comandos[numCommand]->salida);
		printf("Entrada personalizada: %s \n",
		       cmds->comandos[numCommand]->here);
		for (int numArg = 0;
		     numArg < cmds->comandos[numCommand]->numArgumentos;
		     numArg++) {
			printf("Argumento %d: %s \n", numArg,
			       cmds->comandos[numCommand]->argumentos[numArg]);
		}
	}

}

/*
void sigint_handler(int signum) {
    printf("\nSeñal SIGINT (Ctrl+C) recibida. Liberando memoria...\n");
    free_commands(&Comandos); // Asegúrate de que Comandos sea visible aquí
    exit(atoi(getenv("result")));
}
*/

// ----------------------- FUNCIONES DEDICADAS A LA LECTURA DE LA ENTRADA ---------------------------------------------------------------------

void
proccess_line(Commands *cmds, char *line)
{
	if (cmds->numCommands > 0) {
		free_commands(cmds);
		initializerCommands(cmds);
	}

	formatter(line, cmds);

	if (cmds->numCommands > 0) {
		search_paths(cmds);
		//commands_printer(cmds);
		exec_cmds(cmds);
		//commands_printer(cmds);
	}
}

void
read_lines(Commands *cmds)
{
	char *line = (char *)malloc(LINE_BUFFER_SIZE);

	malloc_check(line);

	int total_size = LINE_BUFFER_SIZE;
	int total_read = 0;	//esto representará lo que hemos leido en total de toda las llamadas a fgets

	//int actual_position = line + total_read; // ESTAMOS AL PRINCIPIO DE line

	while (1) {
		//printf("Tamaño de line: %ld \n", sizeof(line));
		if (fgets(line + total_read, LINE_BUFFER_SIZE, stdin) == NULL) {
			break;	// ERROR O EOF
		}
		//printf("Tamaño de line despues de leer: %ld \n", sizeof(line));
		// fgets empezará a escribir en la dir de memoria line + total_read
		// leera como maximo LINE_BUFFER_SIZE
		// lo leera de stdin

		size_t last_read = strlen(line + total_read);	// ESTA VARIABLE NOS DICE, CUANTO TEXTO HEMOS LEIDO EN LA ULTIMA INTERVENCION Y LE QUITAMOS 1 DEL \n

		//total_read = total_read + last_read;

		if (last_read == 0 || line[total_read + last_read - 1] == '\n') {	// SI NO HEMOS LEIDO NADA, O SI LA ULTIMA POSICION ES UNSALTO DE LINEA
			if (line[total_read + last_read - 1] == '\n') {
				// Eliminar el '\n' si se ha leído una línea completa
				clean_line(line, '\n');
				//line[total_read + last_read - 1] = '\0';

			}
			//printf("Leido total: %d \n", total_read);
			//printf("Ultima lectura: %d \n", strlen(line));
			total_read = 0;	// REINICIAMOS PAA LA SIGUIENTE LINEA, POR QUE LA HEMOS LEIDO ENTERA (ESTO SERIA OTRO)

			if (total_read + last_read > 1) {	// COMPROBAMOS QUE HAYA LEÍDO ALGO, (DEBE SER > 1, PORQUE last_read, tiene \n aunque no se lea nada)
				proccess_line(cmds, line);
			}

		} else {
			// SI EL ULTIMO CARACTER NO ES \n SIGNIFICA QUE NO HEMOS LEIDO LA LINEA ENTERA
			total_read += last_read;	// ACTUALIZAMOS total_read CON LO QUE HEMOS LEIDO EN LA ULTIMA LLA,ADA
			total_size += LINE_BUFFER_SIZE;	// SUMAMOS OTROS 256 CARACTERES
			line = realloc(line, total_size);	// Hacemos un realloc con el nuevo tamaño, y continuaremos leyendo desde line + current_size
			malloc_check(line);
		}

	}

	if (!feof(stdin)) {
		// Error al leer
		errx(EXIT_FAILURE, "error reading input");
	}

	free(line);
}

// ----------------------- FIN DE FUNCIONES DEDICADAS A LA LECTURA DE LA ENTRADA ---------------------------------------------------------------------

int
main(int argc, char *argv[])
{

	initialize_result();

	Commands Comandos;

	initializerCommands(&Comandos);

	if (argc > 1) {
		err(EXIT_FAILURE, "No arguments needed.");
	}

	read_lines(&Comandos);

	free_commands(&Comandos);

	exit(atoi(getenv("result")));
	return 0;

}
