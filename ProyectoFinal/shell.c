#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <sys/wait.h>
#include <string.h>
#include <glob.h>

// VFINAL 1.1

/*

gcc -Wall -Wshadow -Wvla -g -c shell.c
gcc -g -o shell shell.o
valgrind --leak-check=yes ./shell

*/

// PARAMETERS AND CONSTANTS
enum {
	MAX_PATH_LENGTH = 256,
	LINE_BUFFER_SIZE = MAX_PATH_LENGTH * sizeof(char),
};

const char *builtin_cmds[] = {
	"cd",
	"=",
	"ifok",
	"ifnot",
};
// END OF PARAMETERS AND CONSTANTS

// DATA STRUCTURES USED
struct Command {
	char *name;
	char *path;
	int numArguments;
	char **arguments;
	char *input;
	char *output;
	char *here;
};

typedef struct Command Command;

struct Commands {

	int numCommands;
	Command **commands;
	int background;

};

typedef struct Commands Commands;
// END OF DATA STRUCTURES USED


// DIFFERENT ERROR MESSAGES
const char *mem_locate = "Error: Could not allocate memory for the argument.\n";
const char *writeFail = "Error: Write failed. \n";
const char *fileNotExist = "The file doesn't exist. \n";
const char *env_var_error = "Error to establish environment variable";
const char *child_process_err = "Theres an error with the child process";
const char *first_pipe_err = "Theres an error creating the first pipe.";
const char *abnormallyError = "The child ended abnormally";
// END OF DIFFERENT ERROR MESSAGES

// SOME BASIC ACTIONS
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
// END OF SOME BASIC ACTIONS

// ERRORS MANAGEMENT
void
memLocateFailed()
{
	err(EXIT_FAILURE, "%s" , mem_locate);
}


int
malloc_check(char *line)
{
	if (line == NULL) {
		memLocateFailed();
        return 0;
	}
	return 1;
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
pipe_malloc_check(const int *pipe)
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
// END OF ERRORS MANAGEMENT

// ENVIRONMENT VARIABLE result INITIALIZATION
void
initialize_result()
{
	setenv("result", "0", 1);
}

// STRUCT COMMAND MANAGEMENT
void
initializerCommand(Command *cmd)
{

	cmd->name = NULL;
	cmd->arguments = NULL;
	cmd->numArguments = 0;
	cmd->path = NULL;
	cmd->input = NULL;
	cmd->output = NULL;
	cmd->here = NULL;

}

char **
reserve_args(Command *cmd)
{
	size_t new_size = (cmd->numArguments + 1) * sizeof(char *);
    void *temp_ptr = realloc(cmd->arguments, new_size);
	if (temp_ptr == NULL) {
		memLocateFailed();
        return NULL;
	}
    cmd->arguments = temp_ptr;
    return cmd->arguments;
}

void
check_empty_name(Command *cmd)
{
	if (cmd->name != NULL) {
		free(cmd->name);
	}
	cmd->name = NULL;
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
	cmd->name = strdup(name);
	malloc_check(cmd->name);
}

void
assignCommandArg(Command *cmd, char *arg)
{
	int numArgs = cmd->numArguments;
	cmd->arguments[numArgs] = strdup(arg);
	malloc_check(cmd->arguments[numArgs]);
}
// END OF STRUCT COMMANDS MANAGEMENT

// STRUCT COMMANDS MANAGEMENT
void
initializerCommands(Commands *cmds)
{

	cmds->numCommands = 0;
	cmds->commands = NULL;
	cmds->background = 0;

}

Command **
reserve_commands(Commands *cmds)
{
	size_t new_size = (cmds->numCommands + 1) * sizeof(Command *);
    void *temp_ptr = realloc(cmds->commands, new_size);
	if (temp_ptr == NULL) {
		memLocateFailed();
        return NULL;
	}
    cmds->commands = temp_ptr;
	cmds->commands[cmds->numCommands] = malloc(sizeof(Command));
	Command *cmd = cmds->commands[cmds->numCommands];
	cmd_malloc_check(cmd);
	initializerCommand(cmd);
	return cmds->commands;
}

void
setLastArgumentNull(Command *cmd)
{
	malloc_check(*reserve_args(cmd));
	cmd->arguments[cmd->numArguments] = NULL;
}
// END OF THE COMMANDS STRUCT MANAGEMENT

// DEDICATED FUNCTIONS TO THE MEMORY LIBERATION ASSIGNED DYNAMICALLY OF THE COMMANDS
void
free_command(Command *cmd)
{
	free(cmd->name);
	free(cmd->path);
	free(cmd->output);
	free(cmd->input);
	free(cmd->here);

	for (int i = 0; i < cmd->numArguments; i++) {

		free(cmd->arguments[i]);

	}
	free(cmd->arguments);
	free(cmd);
}

void
free_commands(Commands *cmds)
{
	int total_cmds = cmds->numCommands;
	for (int numCmd = 0; numCmd < total_cmds; numCmd++) {
		free_command(cmds->commands[numCmd]);
	}
	free(cmds->commands);
}
// END OF DEDICATED FUNCTIONS TO THE FREE MEMORY DYNAMICALLY ASSIGNED

// ASSIGNATION AND SUBSTITUTION OF ENVIRONMENT VARS (SPECIAL COMMAND)
void
add_assignation_arg(char *vars, Command *cmd) {

    if (reserve_args(cmd) == NULL) {
        memLocateFailed();
        return;
    }
    // SAVE THE VALUE OF THE VARIABLE AS THE 0 ARGUMENT
    cmd->arguments[cmd->numArguments++] = strdup(vars);
    malloc_check(cmd->arguments[cmd->numArguments - 1]);
}

void
variable_asig(Commands *cmds, char *token)	
{
	char *vars;
	char *saveptr;

	Command *cmd = cmds->commands[cmds->numCommands];
	assignCommandName(cmd, "=");
	vars = strtok_r(token, "=", &saveptr);
    add_assignation_arg(vars, cmd);
	vars = strtok_r(NULL, " ", &saveptr);	
	if (vars != NULL) {
        add_assignation_arg(vars, cmd);
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
write_new_token(char **new_token, char *variable, char *original_token_copy)
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
    // NOW WE HAVE TO RECONSTRUCT THE TOKEN, WITH THE DELIMITED VARIABLE
	size_t mem_size = strlen(variable) + strlen(original_token_copy) + 2;
	*new_token = malloc(mem_size);
	if (*new_token != NULL) {
        write_new_token(*&new_token, variable, original_token_copy);
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
// END OF ASSIGNATION AND SUBSTITUTION OF ENVIRONMENT VARS (SPECIAL COMMAND)

// LOGIC FOR THE OPTIONAL III (GLOBBING IMPLEMENTATION)
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
// END OF OPTIONAL III LOGIC (GLOBBING IMPLEMENTATION)

// LOGIC FOR THE OPTIONAL I (HERE DOCUMENT IMPLEMENTATION)
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
    char *line = NULL;
	line = (char *)malloc(LINE_BUFFER_SIZE);
	malloc_check(line);
	size_t total_size = LINE_BUFFER_SIZE;
    size_t total_read = 0;

	do {
        if (line != NULL) {
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
	}
	while (strchr(line, '}') == NULL);	

	clean_line(line, '}');
	cmds->commands[0]->here = strdup(line);
	malloc_check(cmds->commands[0]->here);
	free(line);
}
// END OF OPTIONAL I LOGIC (HERE DOCUMENT IMPLEMENTATION)

// FUNCTIONS FOR TOKEN THE INPUT AND SET COMMANDS AND HIS RESPECTIVE ARGUMENTS
void
instruction_pipe(Commands *cmds, char **token, char **saveptr)
{

	setLastArgumentNull(cmds->commands[cmds->numCommands]);
	cmds->numCommands++;

	if (reserve_commands(cmds) == NULL) {
		memLocateFailed();
		return;
	}

	*token = strtok_r(NULL, " ", *&saveptr);
	assignCommandName(cmds->commands[cmds->numCommands], *token);
}

void
instruction_output_redirection(Commands *cmds, char **saveptr)
{
	char *file_name = strtok_r(NULL, " ", *&saveptr);
	Command *cmd = cmds->commands[cmds->numCommands];
	cmd->output = strdup(file_name);
	malloc_check(cmd->output);
}

void
instruction_input_redirection(Commands *cmds, char **saveptr)
{
	char *file_name = strtok_r(NULL, " ", *&saveptr);
	Command *cmd = cmds->commands[0];
	cmd->input = strdup(file_name);
	malloc_check(cmd->input);
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
	Command *cmd = cmds->commands[numPipeCmd];
	if (cmd->name != NULL) {
		if (reserve_args(cmd) == NULL) {
			memLocateFailed();
			return;
		}
		assignCommandArg(cmd, *token);
		cmd->numArguments++;
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
        instruction_output_redirection(cmds, saveptr);
	} else if (strcmp(*token, "<") == 0) {
        instruction_input_redirection(cmds, saveptr);
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
		cmd = cmds->commands[cmds->numCommands];
		if (cmd->numArguments != 0) {
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

// FUNCTIONS DEDICATED TO FORMAT THE INPUT BEFORE PASS TO THE TOKENIZATION
void
rewrite_line(const char *line, char *nuevoString)
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

	size_t length = strlen(line);
	size_t new_size = (2 * length + 1) * sizeof(char);
	char *line_no_tabs = (char *)malloc(new_size);
	malloc_check(line_no_tabs);

	char *nuevoString = (char *)malloc(new_size);
	malloc_check(nuevoString);

	remove_tabs(line, line_no_tabs);	
	rewrite_line(line_no_tabs, nuevoString);	


	if (nuevoString != NULL) {
		tokenizator(nuevoString, cmds);
		free(nuevoString);
		free(line_no_tabs);
	} else {
		memLocateFailed();
	}
}
// END OF FUNCTIONS DEDICATED TO FORMAT THE INPUT BEFORE THE TOKENIZATION

// FUNCTIONS DEDICATED TO THE EXECUTABLES SEARCH IN DIFFERENT PATHS
void
create_current_dir(char *token, Command *cmd, char **current_dir)
{

	size_t new_size = strlen(token) + 2 + strlen(cmd->name);
	*current_dir = malloc(new_size);	
	malloc_check(*current_dir);

	strcpy(*current_dir, token);
	strcat(*current_dir, "/");
	strcat(*current_dir, cmd->name);

}

int
access_dir(char *current_dir, Command *cmd)
{			
	int found = 0;
	if (access(current_dir, F_OK) == 0) {
		if (cmd->path == NULL) {
			free(cmd->path);
		}
		cmd->path = strdup(current_dir);
		malloc_check(cmd->path);
		found = 1;
	}
	return found;
}

void
searchin_paths(Command *cmd)
{
	char *sh_paths = getenv("PATH");	
	char *sh_paths_copy = strdup(sh_paths);
	int program_found = 0;

	malloc_check(sh_paths_copy);

	char *token;
	char *saveptr;
	char *current_dir;

	token = strtok_r(sh_paths_copy, ":", &saveptr);

	while ((token != NULL) && (program_found == 0)) {

        create_current_dir(token, cmd, &current_dir);
		program_found = access_dir(current_dir, cmd);
		free(current_dir);
		token = strtok_r(NULL, ":", &saveptr);

	}
	free(sh_paths_copy);
}

void
searchin_pwd(Command *cmd)
{
	if (access(cmd->name, F_OK) == 0) {
		searchin_paths(cmd);
		cmd->path = strdup(cmd->name);
		
	} else {
		searchin_paths(cmd);
	}
}

int
is_builtin(Command *cmd)
{			
	int found = 0;

	for (int i = 0; builtin_cmds[i] != NULL; i++) {
		if (strcmp(cmd->name, builtin_cmds[i]) == 0) {
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
		printf("Command %s not found. \n", cmd->name);
		setenv("result", "1", 1);
	}
}

void
search_paths(Commands *cmds)
{
    for (int numCmd = 0; numCmd < cmds->numCommands; numCmd++) {
        search_path(cmds->commands[numCmd]);
    }
}
// END OF DEDICATED FUNCTIONS FOR THE EXECUTABLES SEARCH IN DIFFERENT PATHS

// LOGIC FOR REDIRECTIONS

void
fd_setter(Command *cmd, int *fd_in, int *fd_out)	
{

	if (cmd->input != NULL) {
		*fd_in = open(cmd->input, O_RDONLY);
		if (open_check(*fd_in) == 0) {
			dup2(*fd_in, STDIN_FILENO);
		} else {
			free(cmd->input);
			cmd->input = NULL;
		}
	} else {
		*fd_in = STDIN_FILENO;
	}

	if (cmd->output != NULL) {
		*fd_out = open(cmd->output, O_CREAT | O_WRONLY | O_TRUNC, 0666);
		if (open_check(*fd_out) == 0) {
			dup2(*fd_out, STDOUT_FILENO);
		} else {
			free(cmd->output);
			cmd->output = NULL;
		}
	} else {
		*fd_out = STDOUT_FILENO;
	}
}

// END OF REDIRECTIONS LOGIC


// PIPES EXECUTION AND INDIVIDUAL COMMANDS
void
wait_single_child()
{
	int status;
	wait(&status);

	// ............ ENV VAR "result" ..................
	char wexit[2];		

	if (WIFEXITED(status)) {
		// ......... For env var "result" .....................
		sprintf((char *)wexit, "%d", WEXITSTATUS(status));	
		setenv("result", wexit, 1);
		// .....................................................
	} else {
		err(EXIT_FAILURE, "%s", abnormallyError);
	}
	// ............................................
}

void
execute_pipe(Commands *cmds)
{
    int numPipeCmd = cmds->numCommands - 1;
	size_t new_size = numPipeCmd * sizeof(int *);
	int **pipes = malloc(new_size);

	for (int pipe = 0; pipe < numPipeCmd; pipe++) {
		pipes[pipe] = malloc(2 * sizeof(int));
		pipe_malloc_check(pipes[pipe]);
	}

	int numCmds = cmds->numCommands;
	for (int numCmd = 0; numCmd < numCmds; numCmd++) {
		Command *cmd = cmds->commands[numCmd];
		// ....................... IN CASE OF HERE{} ............................
		int pipe_here[2];
		if (cmds->commands[numCmd]->here != NULL) {
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
		// ...................... ENV VAR "result" IN CASE OF & ...............
		switch (fork()) {
            case -1: {
                err(EXIT_FAILURE, "%s" , child_process_err);
            }
            case 0: {
                int fd_in;
                int fd_out;
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
                // ......... IN CASE OF HERE{} .................
                if (cmd->here != NULL) {
                    close(pipe_here[1]);
                    if (dup2(pipe_here[0], fd_in) == -1) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(pipe_here[0]);
                }
                // ..............................................
                execv(cmd->path, cmd->arguments);
                printf("There's an error executing the command %s. \n", cmd->name);
                exit(EXIT_FAILURE);
            }
            default: {
                // ............ IN CASE OF HERE{} ...................
                if (cmd->here != NULL) {
                    close(pipe_here[0]);
                    write(pipe_here[1], cmd->here, strlen(cmd->here));
                    close(pipe_here[1]);
                }
                // ...........................................................

                if (numCmd < numPipeCmd) {
                    close(pipes[numCmd][1]);
                }
            }
		}
	}

	if (cmds->background == 0) {
		for (int i = 0; i < numCmds; i++) {
			int status;
			wait(&status);
			// ........... ENV VAR result (last value will be set)..........
			char wexit[2];
			if (WIFEXITED(status)) {
				// ......... For env var "result" .....................
				sprintf((char *)wexit, "%d", WEXITSTATUS(status));	
				setenv("result", wexit, 1);
				// .........................................................
			} else {
				err(EXIT_FAILURE, "%s", abnormallyError);
			}
			// ....................................................
		}
	}
	// ------ FREE MEMORY ASSIGNED TO PIPES -----------------
	for (int pipe = 0; pipe < numPipeCmd; pipe++) {
		free(pipes[pipe]);
	}
	free(pipes);
	// --------------------------------------------------------
	
}

void
exec_cmd(Command *cmd, int background)
{

	if (cmd->path != NULL) {
		// ................. IN CASE OF HERE ..................
		int pipe_here[2];
		create_here_pipes(cmd, pipe_here);	
		// ............ ENV VAR "result" IN CASE OF & ...............
	
		switch (fork()) {
            case -1: {
                err(EXIT_FAILURE, "Theres an error with the child");
            }

            case 0: {
                int fd_in, fd_out;
                fd_setter(cmd, &fd_in, &fd_out);
                if (fd_in >= 0 && fd_out >= 0) {
                    execv(cmd->path, cmd->arguments);
                }
                exit(1);
            }
            default: {
                // ....... IN CASE OF HERE{} ...............
                close_here_father_pipes(cmd, pipe_here);
                // ......................................
                if (background == 0) {
                    wait_single_child();
                }
            }
		}
	} else {
		setenv("result", "1", 1);
	}
}
// END OF PIPES AND INDIVIDUAL COMMANDS EXECUTION

// BUILT-IN COMMANDS
void
exec_cd(Command *cmd)
{
	// ......... For env var "result" ...........
	char wexit[2];		
	int status = 0;
	// .......................................

	if (cmd->numArguments > 1) {
		char *dir = cmd->arguments[1];
		if (chdir(dir) != 0) {
			status = 1;
			printf("The dir doesn't exist. \n");
		}
	} else {
		char *sh_home = getenv("HOME");
		chdir(sh_home);
	}

	// ............. Define of env var "result" .....................
	sprintf((char *)wexit, "%d", status);
	setenv("result", wexit, 1);	
	// ...............................................................
}

void
exec_asig(Command *cmd)
{

	// ......... For env var "result" .....................
	char wexit[2];		
	int status = 0;
	// ..................................................

	if (setenv(cmd->arguments[0], cmd->arguments[1], 1) != 0) {
        err(EXIT_FAILURE, "%s", env_var_error);
	}
	char *env_value = getenv(cmd->arguments[0]);

	if (env_value == NULL) {
		status = 1;
		printf("%s not defined.\n", cmd->arguments[0]);
	}
	// ............. Define of env var "result" .....................
	sprintf((char *)wexit, "%d", status);
	setenv("result", wexit, 1);	
	// ...............................................................

}

void
exec_sust(Command *cmd)
{			
	// ......... For env var "result" .....................
	char wexit[2];		
	int status = 0;
	// ..................................................

	char *var = cmd->arguments[0];
	char *variable = getenv(var);
	if (variable == NULL) {
		status = 1;
		printf("error: var %s does not exist. \n", var);
	} else {
		printf("%s \n", variable);
	}
	// ............. Define of env var "result" .....................
	sprintf((char *)wexit, "%d", status);
	setenv("result", wexit, 1);
	// ...............................................................

}

void
remake_cmd(Command *cmd, int *background)
{		
	check_empty_path(cmd);
	assignCommandName(cmd, cmd->arguments[1]);

	int numArgs = cmd->numArguments;
	for (int i = 1; i < numArgs; i++) {
		if (strcmp(cmd->arguments[i], "&") == 0) {
			*background = 1;
		}
		if (cmd->arguments[i - 1] != NULL) {
			free(cmd->arguments[i - 1]);
		}
		cmd->arguments[i - 1] = strdup(cmd->arguments[i]);
	}

	cmd->numArguments = cmd->numArguments - 1;
	free(cmd->arguments[cmd->numArguments]);

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
// END OF BUILT-IN COMMANDS

// COMMANDS EXECUTION LOGIC
void
exec_builtin(Command *cmd)
{
	if (strcmp(cmd->name, "cd") == 0) {
		exec_cd(cmd);
	}
	if (strcmp(cmd->name, "=") == 0) {
		exec_asig(cmd);
	}
	if (strcmp(cmd->name, "$") == 0) {
		exec_sust(cmd);
	}
	if (strcmp(cmd->name, "ifok") == 0) {
		exec_ifok(cmd);
	}
	if (strcmp(cmd->name, "ifnot") == 0) {
		exec_ifnot(cmd);
	}
}

int
check_all_paths(Commands *cmds)
{				
	int path = 0;		
	int numPipeCmd = cmds->numCommands;
	for (int numCmd = 0; numCmd < numPipeCmd; numCmd++) {
		if (cmds->commands[numCmd]->path == NULL) {
			path = 1;
		}
	}
	return path;
}

void
exec_single(Command *cmd, int is_background)
{
    if (is_builtin(cmd) == 1) {
        exec_builtin(cmd);
    } else {
        exec_cmd(cmd, is_background);
    }
}

void
exec_cmds(Commands *cmds)
{
	if (cmds->numCommands > 1) {
		if (check_all_paths(cmds) == 0) {
			execute_pipe(cmds);
		}
	} else {
        Command *cmd= cmds->commands[0];
        int is_background = cmds->background;
        exec_single(cmd, is_background);
	}

}
// END OF COMMANDS EXECUTION LOGIC

// FUNCTIONS DEDICATED IN INPUT READING
void
process_line(Commands *cmds, char *line)
{
	if (cmds->numCommands > 0) {
		free_commands(cmds);
		initializerCommands(cmds);
	}
	formatter(line, cmds);
    search_paths(cmds);
    //commands_printer(cmds);
    exec_cmds(cmds);
}

void
read_lines(Commands *cmds)
{
	char *line = (char *)malloc(LINE_BUFFER_SIZE);
	malloc_check(line);

	int total_size = LINE_BUFFER_SIZE;
	size_t total_read = 0;

	while (1) {
        if (line != NULL) {
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
                    process_line(cmds, line);
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
	}

	if (!feof(stdin)) {
		errx(EXIT_FAILURE, "error reading input");
	}

	free(line);
}

// END OF FUNCTIONS DEDICATED IN INPUT READING
int
main(int argc, char *argv[])
{
	initialize_result();
	Commands commands;
	initializerCommands(&commands);
	if (argc > 1) {
		err(EXIT_FAILURE, "No arguments needed.");
	}
	read_lines(&commands);
	free_commands(&commands);
	exit(atoi(getenv("result")));
}