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

// VFINAL 1.0

/*

gcc -Wall -Wshadow -Wvla -g -c proyect.c
gcc -g -o proyect proyect.o
valgrind --leak-check=yes ./proyect

*/

// ----------------------- PARAMETROS Y CONSTANTES -------------------------------------------------

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

// ----------------------- FIN DE PARAMETROS Y CONSTANTES --------------------------------------------

// ----------------------- ESTRUCTURAS DE DATOS UTILIZADAS --------------------------------------------

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

// ----------------------- FIN DE ESTRUCTURAS DE DATOS UTILIZADAS -------------------------------------------


// ------------------------- MENSAJES VARIOS DE ERROR ------------------------------------------------------

const char *memAlocate = "Error: Could not allocate memory for the argument.\n";
const char *writeFail = "Error: Write failed. \n";
const char *fileNotExist = "The file doesn't exist. \n";
const char *env_var_error = "Error to establish environment variable";
const char *child_proccess_err = "Theres an error with the child proccess";
const char *first_pipe_err = "Theres an error creating the first pipe.";
const char *anormaly_error = "The child ended unnormally";

// ------------------------- FIN DE MENSAJES VARIOS DE ERROR  ----------------------------------------------


// ------------------------ ALGUNAS ACCIONES BÁSICAS ---------------------------------------------------------

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

// ------------------------- FIN DE ALGUNAS ACCIONES BASICAS ---------------------------------------------------

// -------------------- GESTIÓN DE ERRORES DEL PROGRAMA --------------------------------------------------------

void
memLocateFailed()
{
	err(EXIT_FAILURE, "%s" ,memAlocate);
}


int
malloc_check(char *line)
{
	if (line == NULL) {
		memLocateFailed();
	}
	return 0;
}

int
cmd_malloc_check(Command *cmd)
{

	if (cmd == NULL) {
		memLocateFailed();	
	}
	return 0;
}

void
write_failed()
{
	err(EXIT_FAILURE, "%s" ,writeFail);
}


void
pipe_malloc_check(int *pipe)
{

	if (pipe == NULL) {
		memLocateFailed();	
	}

}

int
open_check(int fd)
{
	int opened = 0;

	if (fd < 0) {
		printf("%s", fileNotExist);
		opened = 1;
	}

	return opened;
}

// ---------------------- FIN DE GESTIÓN DE ERRORES DEL PROGRAMA----------------------------------------------------

// ------------------------ INICIALIZACION DE VAR DE ENTORNO result ------------------------------------------------

void
initialize_result()
{
	setenv("result", "0", 1);
}

// -------------------------------------------------------------------------------------------------------------------

// ----------------------------- GESTION DEL STRUCT COMMAND ----------------------------------------------------------

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
	int new_size = (cmd->numArgumentos + 1) * sizeof(char *);
	cmd->argumentos = realloc(cmd->argumentos, new_size);
	if (cmd->argumentos == NULL) {
		memLocateFailed();
	}
	return cmd->argumentos;
}

void
check_empty_name(Command *cmd)
{
	if (cmd->nombre != NULL) {	
		free(cmd->nombre);
	}
	cmd->nombre = NULL;
}

void
check_empty_path(Command *cmd)
{
	if (cmd->path != NULL) {	
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
	int numArgs = cmd->numArgumentos;
	cmd->argumentos[numArgs] = strdup(arg);
	malloc_check(cmd->argumentos[numArgs]);
}

// ----------------------------- FIN DE GESTION DEL STRUCT COMMAND ---------------------------------------------------

// ---------------------------- GESTION DEL STRUCT COMMANDS ----------------------------------------------------------

void
initializerCommands(Commands *cmds)
{

	cmds->numCommands = 0;
	cmds->comandos = NULL;
	cmds->background = 0;

}

Command **
reserve_commands(Commands *cmds)
{
	int new_size = (cmds->numCommands + 1) * sizeof(Command *);
	cmds->comandos = realloc(cmds->comandos, new_size);
	if (cmds->comandos == NULL) {
		memLocateFailed();
	}
	
	cmds->comandos[cmds->numCommands] = malloc(sizeof(Command));
	Command *cmd = cmds->comandos[cmds->numCommands];
	cmd_malloc_check(cmd);

	initializerCommand(cmd);	

	return cmds->comandos;	

}

void
setLastArgumentNull(Command *cmd)
{
	malloc_check(*reserve_args(cmd));
	cmd->argumentos[cmd->numArgumentos] = NULL;	
}

// ----------------------------- FIN DE GESTION DEL STRUCT COMMANDS -------------------------------------------------
// ----------- FUNCIONES DEDICADAS A LA LIBERACIÓN DE MEMORIA ASIGNADA DINAMICAMENTE DE LOS COMANDOS ---------------

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
	int total_cmds = cmds->numCommands;
	for (int numCmd = 0; numCmd < total_cmds; numCmd++) {
		free_command(cmds->comandos[numCmd]);
	}
	free(cmds->comandos);
}

// -------------- FIN DE FUNCIONES DEDICADAS A LA LIBERACIÓN DE MEMORIA ASIGNADA DINAMICAMENTE ------------------

// --------------------- ASIGNACION Y SUSTITUCION DE VARIABLES DE ENTORNO (COMANDO ESPECIAL) -------------------
void
add_asignation_arg(char *vars, Command *cmd) {

    if (reserve_args(cmd) == NULL) {
        memLocateFailed();
        return;
    }
    // GUARDAMOS EL VALOR DE LA VARIABLE COMO ARGUMENTO 0
    cmd->argumentos[cmd->numArgumentos++] = strdup(vars);
    malloc_check(cmd->argumentos[cmd->numArgumentos - 1]);

}

void
variable_asig(Commands *cmds, char *token)	
{
	char *vars;
	char *saveptr;

	Command *cmd = cmds->comandos[cmds->numCommands];

	assignCommandName(cmd, "=");
	vars = strtok_r(token, "=", &saveptr);	
	add_asignation_arg(vars, cmd);

	vars = strtok_r(NULL, " ", &saveptr);	
	if (vars != NULL) {
		add_asignation_arg(vars, cmd);
	}

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
		variable = "";	
	}

	return variable;
}

void
write_newtoken(char **new_token, char *variable, char *original_token_copy)
{

	if (strcmp(variable, "") == 0) {
		strcpy(*new_token, original_token_copy);
	} else {
		strcpy(*new_token, variable);
		strcat(*new_token, original_token_copy);
	}

}

void
remake_token(char **new_token, char *variable, char *original_token_copy)
{
	// AHORA DEBEMOS RECONSTRUIR EL TOKEN, CON LA VARIABLE DELIMITADA
	int mem_size = strlen(variable) + strlen(original_token_copy) + 2;
	*new_token = malloc(mem_size);
	if (*new_token != NULL) {
		write_newtoken(*&new_token, variable, original_token_copy);
	} else {
		memLocateFailed();
	}

}

void
sust_vars(char **token, char **new_token)
{		

	char *ptr;
	char *rest = NULL;
	char *variable;
	char *delimiter = " /\t";
	char *original_token_copy;

	ptr = strchr(*token, '$');
	ptr++;		
	
	original_token_copy = strdup(ptr);
	malloc_check(original_token_copy);
	rest = get_next_token(ptr, delimiter);	
	
	original_token_copy += strlen(rest);

	if (rest != NULL) {
		variable = get_variable(rest);
		if (*original_token_copy == '\0') {
			*new_token = strdup(variable);
			malloc_check(*new_token);
		} else {
			remake_token(&*new_token, variable, original_token_copy);	
		}
	} else {
		*new_token = strdup("$");
		malloc_check(*new_token);
	}

	if (original_token_copy != NULL) {
		free(original_token_copy - strlen(rest));	
	}

}

// ---- FIN DE ASIGNACION Y SUSTITUCION DE VARIABLES DE ENTORNO (COMANDO ESPECIAL) ---------------------

// ---------------- LOGICA PARA EL OPCIONAL III (Implementacion de globbing) -------------------------------

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
	
	int return_value;	
	return_value = glob(token, 0, NULL, glob_result);
	return return_value;

}

// --------------- FIN LOGICA PARA EL OPCIONAL III (Implementacion de globbing) ----------------

// ------------- LOGICA PARA EL OPCIONAL I (IMPLEMENTACION DE HERE{}) ---------------------------

void
create_here_pipes(Command *cmd, int pipe_here[2])
{
	if (cmd->here != NULL) {
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
		close(pipe_here[1]);
		if (dup2(pipe_here[0], input) == -1) {	
			perror("dup2");
			exit(EXIT_FAILURE);
		}
		close(pipe_here[0]);	
	}
}

void
close_here_father_pipes(Command *cmd, int pipe_here[2])
{
	if (cmd->here != NULL) {
		close(pipe_here[0]);
		if (write(pipe_here[1], cmd->here, strlen(cmd->here)) == -1) {
			write_failed();
		}		
		close(pipe_here[1]);	
	}
}

void
first_optional(Commands *cmds)
{
	char *line = (char *)malloc(LINE_BUFFER_SIZE);	
	malloc_check(line);
	int total_size = LINE_BUFFER_SIZE;
	int total_read = 0;	

	do {
		if (fgets(line + total_read, LINE_BUFFER_SIZE, stdin) == NULL) {
			break;	
		}
		
		size_t last_read = strlen(line + total_read);	
		total_read += last_read;	
		total_size += LINE_BUFFER_SIZE;	
		char *temp = realloc(line, total_size);

		if (temp == NULL) {
			free(line);
			line = NULL;
			malloc_check(temp);
		} else {
			line = temp;
		}
	}
	while (strchr(line, '}') == NULL);	

	clean_line(line, '}');
	cmds->comandos[0]->here = strdup(line);
	malloc_check(cmds->comandos[0]->here);
	free(line);
}

// ---------- FIN LOGICA PARA EL OPCIONAL I (IMPLEMENTACION DE HERE{}) ------------------------------------------------------

// -------------- FUNCIONES PARA TOKENIZAR ENTRADA Y SETEAR COMANDOS Y SUS RESPECTIVOS ARGUMENTOS ---------------------------

void
instruction_pipe(Commands *cmds, char **token, char **saveptr)
{

	setLastArgumentNull(cmds->comandos[cmds->numCommands]);
	cmds->numCommands++;

	if (reserve_commands(cmds) == NULL) {
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
	Command *cmd = cmds->comandos[cmds->numCommands];
	cmd->salida = strdup(file_name);
	malloc_check(cmd->salida);
}

void
instruction_input_redirection(Commands *cmds, char **token, char **saveptr)
{
	char *file_name = strtok_r(NULL, " ", *&saveptr);
	Command *cmd = cmds->comandos[0];
	cmd->entrada = strdup(file_name);
	malloc_check(cmd->entrada);
}

void
instruction_here(Commands *cmds)
{
	if (cmds->background == 0) {	
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

	int numPipeCmd = cmds->numCommands;
	Command *cmd = cmds->comandos[numPipeCmd];
	if (cmd->nombre != NULL) {
		if (reserve_args(cmd) == NULL) {
			memLocateFailed();
			return;
		}
		assignCommandArg(cmd, *token);
		cmd->numArgumentos++;
	} else {
		assignCommandName(cmd, *token);
	}

}

void
instruction_classifier(Commands *cmds, char **token, char **saveptr)
{

	if (strcmp(*token, "|") == 0) {
		instruction_pipe(cmds, token, saveptr);
	} else if (strcmp(*token, ">") == 0) {
		instruction_output_redirection(cmds, token, saveptr);
	} else if (strcmp(*token, "<") == 0) {
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
		sust_vars(*&token, &new_token);
		instruction_classifier(cmds, &new_token, &saveptr);
		free(new_token);
		return 1;
	} else {
		return 0;
	}
}

void
globbing_classifier(Commands *cmds, char *token, char *saveptr)
{
	glob_t glob_result;	
	int patterns_found = check_glob(token, &glob_result);

	if (patterns_found == 0) {
		for (int i = 0; i < glob_result.gl_pathc; i++) {
			token = glob_result.gl_pathv[i];
			instruction_classifier(cmds, &token, &saveptr);
		}
	}
	globfree(&glob_result);	
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
	int is_envar, is_glob;
	Command *cmd = NULL;

	if (reserve_commands(cmds) == NULL) {
		memLocateFailed();
		return;
	}

	token = strtok_r(line, " ", &saveptr);

	while (token != NULL) {
		cmd = cmds->comandos[cmds->numCommands];
		if (cmd->numArgumentos != 0) {
			token = strtok_r(NULL, " ", &saveptr);
		}
		if (token != NULL) {
			is_envar = envar_detector(cmds, &token, saveptr);
			is_glob = glob_detector(cmds, &token, saveptr);
			if ((is_envar || is_glob) == 0) {
				instruction_classifier(cmds, &token, &saveptr);
			}
		}
	}

	setLastArgumentNull(cmd);
	cmds->numCommands++;

}

// -------------- FUNCIONES DEDICADAS A FORMATEAR LA ENTRADA ANTES DE PASARLA AL TOKENIZADOR --------------------------

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
	int new_size = (2 * longitud + 1) * sizeof(char);
	char *linenotabs = (char *)malloc(new_size);
	malloc_check(linenotabs);

	char *nuevoString = (char *)malloc(new_size);
	malloc_check(nuevoString);

	remove_tabs(line, linenotabs);	
	rewrite_line(linenotabs, nuevoString);	


	if (nuevoString != NULL) {
		tokenizator(nuevoString, cmds);
		free(nuevoString);
		free(linenotabs);
	} else {
		memLocateFailed();
	}
}

// ---------------FIN DE FUNCIONES DEDICADAS A FORMATEAR LA ENTRADA ANTES DE PASARLA AL TOKENIZADOR --------------

// ---------------- FUNCIONES DEDICADAS A LA BUSCADA DE EJECUTABLES EN DIFERENTES PATHS ----------------------------

void
create_currentdir(char *token, Command *cmd, char **current_dir)
{				

	int new_size = strlen(token) + 2 + strlen(cmd->nombre);
	*current_dir = malloc(new_size);	
	malloc_check(*current_dir);

	strcpy(*current_dir, token);
	strcat(*current_dir, "/");
	strcat(*current_dir, cmd->nombre);

}

void
access_dir(char *current_dir, Command *cmd)
{			
	if (access(current_dir, F_OK) == 0) {
		if (cmd->path != NULL) {
			free(cmd->path);
		}
		cmd->path = strdup(current_dir);
		malloc_check(cmd->path);
	}
}

void
searchin_paths(Command *cmd)
{
	char *sh_paths = getenv("PATH");	
	char *sh_paths_copy = strdup(sh_paths);

	malloc_check(sh_paths_copy);

	char *token;
	char *saveptr;
	char *current_dir;

	token = strtok_r(sh_paths_copy, ":", &saveptr);

	while (token != NULL) {

		create_currentdir(token, cmd, &current_dir);
		access_dir(current_dir, cmd);
		free(current_dir);
		token = strtok_r(NULL, ":", &saveptr);

	}
	free(sh_paths_copy);
}

void
searchin_pwd(Command *cmd)
{
	if (access(cmd->nombre, F_OK) == 0) {
		searchin_paths(cmd);
		cmd->path = strdup(cmd->nombre);	
		
	} else {
		searchin_paths(cmd);
	}
}

int
is_builtin(Command *cmd)
{			
	int found = 0;

	for (int i = 0; builtin_cmds[i] != NULL; i++) {
		if (strcmp(cmd->nombre, builtin_cmds[i]) == 0) {
			found = 1;
			if (cmd->path != NULL) {
				free(cmd->path);	
			}
			cmd->path = strdup("built-in");
			malloc_check(cmd->path);
		}
	}
	return found;
}

void
search_path(Command *cmd)
{

	if (is_builtin(cmd) != 1) {
		searchin_pwd(cmd);	
	}

	if (cmd->path == NULL) {
		printf("Command %s not found. \n", cmd->nombre);
		setenv("result", "1", 1);
	}
}

void
search_paths(Commands *cmds)
{
	for (int numCmd = 0; numCmd < cmds->numCommands; numCmd++) {
		search_path(cmds->comandos[numCmd]);
	}
}

// ----- FIN DE FUNCIONES DEDICADAS A LA BUSCADA DE EJECUTABLES EN DIFERENTES PATHS -------------------

// ------ LOGICA PARA REDIRECCIONES --------------------------------------------------------------------

void
fd_setter(Command *cmd, int *fd_in, int *fd_out)	
{

	if (cmd->entrada != NULL) {
		*fd_in = open(cmd->entrada, O_RDONLY);
		if (open_check(*fd_in) == 0) {
			dup2(*fd_in, STDIN_FILENO);
		} else {
			free(cmd->entrada);
			cmd->entrada = NULL;
		}
	} else {
		*fd_in = STDIN_FILENO;
	}

	if (cmd->salida != NULL) {
		*fd_out = open(cmd->salida, O_CREAT | O_WRONLY | O_TRUNC, 0666);
		if (open_check(*fd_out) == 0) {
			dup2(*fd_out, STDOUT_FILENO);
		} else {
			free(cmd->salida);
			cmd->salida = NULL;
		}

	} else {
		*fd_out = STDOUT_FILENO;
	}

}

// ------------------------ FIN DE LOGICA PARA REDIRECCIONES -------------------------------------


// ------------------------ EJECUCION DE PIPES Y COMANDOS INDIVIDUALES ----------------------------

void
wait_single_child()
{
	int status;
	wait(&status);

	// ............ ENV VAR "result" ..................
	char wexit[2];		

	if (WIFEXITED(status)) {
		// ......... Para env var "result" .....................
		sprintf((char *)wexit, "%d", WEXITSTATUS(status));	
		setenv("result", wexit, 1);
		// .....................................................
	} else {
		err(EXIT_FAILURE, "%s", anormaly_error);
	}
	// ............................................
}

void
execute_pipe(Commands *cmds)
{

	int child;
	int numPipeCmd = cmds->numCommands - 1;
	int new_size = numPipeCmd * sizeof(int *);
	int **pipes = malloc(new_size);

	for (int pipe = 0; pipe < numPipeCmd; pipe++) {
		pipes[pipe] = malloc(2 * sizeof(int));
		pipe_malloc_check(pipes[pipe]);
	}

	int numCmds = cmds->numCommands;
	for (int numCmd = 0; numCmd < numCmds; numCmd++) {
		Command *cmd = cmds->comandos[numCmd];
		// ....................... EN CASO DE HERE{} ............................
		int pipe_here[2];
		if (cmds->comandos[numCmd]->here != NULL) {
			if (pipe(pipe_here) == -1) {
				perror("pipe");
				exit(EXIT_FAILURE);
			}
		}
		// .........................................................................
		if (numCmd < numPipeCmd) {
			if (pipe(pipes[numCmd]) == -1) {
				err(EXIT_FAILURE, "%s", first_pipe_err);
			}
		}
		// ...................... ENV VAR "result" EN CASO DE QUE & ...............

		switch (child = fork()) {	
		case -1:
			err(EXIT_FAILURE, "%s" ,child_proccess_err);
		case 0:
			int fd_in, fd_out;
			fd_setter(cmd, &fd_in, &fd_out);

			if (numCmd > 0) {	
				dup2(pipes[numCmd - 1][0], fd_in);	
				close(pipes[numCmd - 1][0]);	
				close(pipes[numCmd - 1][1]);
			}
			if (numCmd < numPipeCmd) {
				close(pipes[numCmd][0]);	
				dup2(pipes[numCmd][1], fd_out);	
				close(pipes[numCmd][1]);	
			}
			// ......................... EN CASO DE HERE{} ......................
			if (cmd->here != NULL) {
				close(pipe_here[1]);	
				if (dup2(pipe_here[0], fd_in) == -1) {
					perror("dup2");
					exit(EXIT_FAILURE);
				}
				close(pipe_here[0]);	
			}
			// .....................................................................
			execv(cmd->path, cmd->argumentos);
			printf("There's an error executing the comand %s. \n", cmd->nombre);
			exit(EXIT_FAILURE);
		default:

			// ......................... EN CASO DE HERE{} ..........................
			if (cmd->here != NULL) {
				close(pipe_here[0]);
				write(pipe_here[1], cmd->here, strlen(cmd->here));	
				close(pipe_here[1]);	
			}
			// ......................................................................

			if (numCmd < numPipeCmd) {
				close(pipes[numCmd][1]);
			}

		}
	}

	if (cmds->background == 0) {
		for (int i = 0; i < numCmds; i++) {
			int status;

			wait(&status);
		
			// ........... ENV VAR result (se seteará el ultimo valor)..........
			char wexit[2];	

			if (WIFEXITED(status)) {
				
				// ......... Para env var "result" .....................
				sprintf((char *)wexit, "%d", WEXITSTATUS(status));	
				setenv("result", wexit, 1);
				// .........................................................
			} else {
				err(EXIT_FAILURE, "%s", anormaly_error);
			}
			// ....................................................
		}
	}
	// ------------ LIBEREMOS MEMORIA ASIGNADA A LOS PIPES ----------------------------
	for (int pipe = 0; pipe < numPipeCmd; pipe++) {
		free(pipes[pipe]);
	}
	free(pipes);
	// ---------------------------------------------------------------------------
	
}

void
exec_cmd(Command *cmd, int background)
{

	if (cmd->path != NULL) {
		int child;
		// ............................ EN CASO DE HERE ..............................
		int pipe_here[2];
		create_here_pipes(cmd, pipe_here);	
		// .......................... ENV VAR "result" EN CASO DE QUE & ...............
	
		switch (child = fork()) {
		case -1:
			err(EXIT_FAILURE, "Theres an error with the child");
		case 0:
			int fd_in, fd_out;
			fd_setter(cmd, &fd_in, &fd_out);
			// ......................... EN CASO DE HERE{} ..............................
			// ...........................................................................
			if (fd_in >= 0 && fd_out >= 0) {
				execv(cmd->path, cmd->argumentos);
			}
			exit(1);
		default:
			// ......................... EN CASO DE HERE{} ................................
			close_here_father_pipes(cmd, pipe_here);
			// ............................................................................
			if (background == 0) {
				wait_single_child();
			}
		}
	} else {
		setenv("result", "1", 1);
	}

}

// ------------------------ FIN DE EJECUCION DE PIPES Y COMANDOS INDIVIDUALES ------------------------

// ------------------------- COMANDOS BUILT-INS -------------------------------------------------------

void
exec_cd(Command *cmd)
{
	// ......... Para env var "result" .....................
	char wexit[2];		
	int status = 0;
	// ..................................................

	if (cmd->numArgumentos > 1) {
		char *dir = cmd->argumentos[1];         
		if (chdir(dir) != 0) {
			status = 1;
			printf("The dir doesn't exist. \n");
		}
	} else {
		char *sh_home = getenv("HOME");
		chdir(sh_home);
	}

	// ............. Definimos env var "result" .....................
	sprintf((char *)wexit, "%d", status);
	setenv("result", wexit, 1);	
	// ...............................................................
}

void
exec_asig(Command *cmd)
{

	// ......... Para env var "result" .....................
	char wexit[2];		
	int status = 0;
	// ..................................................

	if (setenv(cmd->argumentos[0], cmd->argumentos[1], 1) != 0) {
		status = 1;
		err(EXIT_FAILURE, "%s", env_var_error);
	}
	char *env_value = getenv(cmd->argumentos[0]);

	if (env_value == NULL) {
		status = 1;
		printf("%s not defined.\n", cmd->argumentos[0]);
	}
	// ............. Definimos env var "result" .....................
	sprintf((char *)wexit, "%d", status);
	setenv("result", wexit, 1);	
	// ...............................................................

}

void
exec_sust(Command *cmd)
{			
	// ......... Para env var "result" .....................
	char wexit[2];		
	int status = 0;
	// ..................................................

	char *var = cmd->argumentos[0];
	char *variable = getenv(var);
	if (variable == NULL) {
		status = 1;
		printf("error: var %s does not exist. \n", var);
	} else {
		printf("%s \n", variable);
	}
	// ............. Definimos env var "result" .....................
	sprintf((char *)wexit, "%d", status);
	setenv("result", wexit, 1);
	// ...............................................................

}

void
remake_cmd(Command *cmd, int *background)
{		
	check_empty_path(cmd);
	assignCommandName(cmd, cmd->argumentos[1]);

	int numArgs = cmd->numArgumentos; 
	for (int i = 1; i < numArgs; i++) {
		if (strcmp(cmd->argumentos[i], "&") == 0) {
			*background = 1;
		}
		if (cmd->argumentos[i - 1] != NULL) {
			free(cmd->argumentos[i - 1]);
		}
		cmd->argumentos[i - 1] = strdup(cmd->argumentos[i]);
	}

	cmd->numArgumentos = cmd->numArgumentos - 1;
	free(cmd->argumentos[cmd->numArgumentos]);	

	setLastArgumentNull(cmd);
}

void
exec_ifok(Command *cmd)
{

	char *prev_status = getenv("result");
	if (prev_status == NULL) {
		printf("There's no a prev command status. \n");
	} else {
		if (atoi(prev_status) == 0) {	
			int background = 0;	
			remake_cmd(cmd, &background);
			search_path(cmd);
			exec_cmd(cmd, background);
		}
	}

}

void
exec_ifnot(Command *cmd)	
{

	char *prev_status = getenv("result");
	if (prev_status == NULL) {
		printf("There's no a prev command status. \n");
	} else {
		if (atoi(prev_status) != 0) {
			int background = 0;	
			remake_cmd(cmd, &background);
			search_path(cmd);
			exec_cmd(cmd, background);	
		}
	}

}

// --------------------------- FIN DE COMANDOS BUILT-INS -------------------------------------------------

// ----------------------- LOGICA DE EJECUCIÓN DE COMANDOS -----------------------------------------------

void
exec_builtin(Command *cmd)
{
	if (strcmp(cmd->nombre, "cd") == 0) {
		exec_cd(cmd);
	}
	if (strcmp(cmd->nombre, "=") == 0) {
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
{				
	int path = 0;		
	int numPipeCmd = cmds->numCommands;
	for (int numCmd = 0; numCmd < numPipeCmd; numCmd++) {
		if (cmds->comandos[numCmd]->path == NULL) {
			path = 1;
		}
	}
	return path;
}

void
exec_cmds(Commands *cmds)
{
	if (cmds->numCommands > 1) {
		if (check_all_paths(cmds) == 0) {	
			execute_pipe(cmds);
		}
	} else {
		Command *cmd_name = cmds->comandos[0];
		int is_background = cmds->background;
		if (is_builtin(cmd_name) == 1) {
			exec_builtin(cmd_name);
		} else {
			exec_cmd(cmd_name, is_background);
		}
	}

}

// ----------------------- FIN LOGICA DE EJECUCIÓN DE COMANDOS ------------------------------


// ----------------------- FUNCIONES DEDICADAS A LA LECTURA DE LA ENTRADA ---------------------

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
	}
}

void
read_lines(Commands *cmds)
{
	char *line = (char *)malloc(LINE_BUFFER_SIZE);
	malloc_check(line);

	int total_size = LINE_BUFFER_SIZE;
	int total_read = 0;	

	while (1) {
		char *initial_size = line + total_read;
		char *read = fgets(initial_size, LINE_BUFFER_SIZE, stdin);
		if (read == NULL) {
			break;
		}
		size_t last_read = strlen(line + total_read);

		char last_char = line[total_read + last_read - 1];
		if (last_read == 0 || last_char == '\n') {	
			if (last_char == '\n') {
				clean_line(line, '\n');
			}
			total_read = 0;
			if (total_read + last_read > 1) {	
				proccess_line(cmds, line);
			}

		} else {
			total_read += last_read;
			total_size += LINE_BUFFER_SIZE;	
			char *temp = realloc(line, total_size);
			if (temp == NULL) {
				free(line);
				line = NULL;
				malloc_check(temp);
			} else {
				line = temp;
			}
		}

	}

	if (!feof(stdin)) {
		errx(EXIT_FAILURE, "error reading input");
	}

	free(line);
}

// ----------------------- FIN DE FUNCIONES DEDICADAS A LA LECTURA DE LA ENTRADA ----------------

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