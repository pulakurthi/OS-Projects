#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> 

#define DISPLAY_ERROR	write(STDERR_FILENO, error_message, strlen(error_message));

char error_message[30] = "An error has occurred\n";

char *path[255] = {"/bin/", "/usr/bin/"};
int num_paths = 2;

const char *delimiters = " \t\n";

void parse_line(char *line);
bool check_and_exe_builtin_cmd(char *line);
int decode_command(char *line, char **myargs);
void parse_command(char *line);
bool check_valid_redirection(char *line, char **output_file);
void run_program(char *myargs[], char *output_file);
char* get_program(char *token);


int main(int argc, char *argv[]) {
	/* The shell can be invoked with either no arguments or
	   a single argument; anything else is an error. */
	if(argc > 2){
		DISPLAY_ERROR
		exit(1);
	}

	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen;
	/* interactive mode */
	if(argc == 1) {
		while(1){
			printf("wish> ");
			if ((linelen = getline(&line, &linecap, stdin)) > 0){
				parse_line(line);
			}
		}
	}
	/* batch mode */
	else {
		FILE *fp = fopen(argv[1], "r");

		if (fp == NULL) {
			DISPLAY_ERROR
			exit(1);
		}

		while ((linelen = getline(&line, &linecap, fp)) > 0){
			parse_line(line);
		}

		fclose(fp);
	}

	exit(0);
}


void parse_line(char *line){
	char *dup_line = strdup(line);

	/* Built-in Commands 
	   note: & is not supported for built in commands */
	if (true == check_and_exe_builtin_cmd(dup_line))
	{
		free(dup_line);
		return;
	}
	free(dup_line);

	char *saveptr;
	char *token = strtok_r(line, "&", &saveptr);
	pid_t children[255];
	int num_children = 0;

	while(token != NULL) {
		pid_t pid = fork();
		if(pid == 0) {
			/* child (new process) */
			parse_command(token);
			exit(0);
		} else if(pid > 0) {
			/* parent keeps track of childrun */
			children[num_children++] = pid;
		} else {
			/* fork error */
			exit(1);
		}
		token = strtok_r(NULL, "&", &saveptr);
	}
	/* wait for childrun to complete */
	for(int i = 0; i < num_children; i++) {
		waitpid(children[i], NULL, 0);
	}
}


bool check_and_exe_builtin_cmd(char *line)
{
	bool return_val = true;
	char **myargs = malloc(255 * sizeof(char *));
	int myargc = decode_command(line, myargs);

	if(myargc == 0){
		free(myargs);
		return return_val;
	}

	else if(strcmp(myargs[0], "exit") == 0){
		/* Tries to exit with an argument. */
		if (myargc != 1){
			DISPLAY_ERROR
		}
		else{
			exit(0);
		}
	}

	else if(strcmp(myargs[0], "cd") == 0){
		/* cd always take one argument (0 or >1 args should be signaled as an error). */
		if(myargc != 2 || 0 != chdir(myargs[1])){
			DISPLAY_ERROR
		}
	}

	else if(strcmp(myargs[0], "path") == 0){
		num_paths = myargc - 1;
		for(int p = 0; p < num_paths; p++){
			/* note: path entries are not freed */
			char *new_path = malloc(strlen(myargs[p+1]) + 2);
			/* malloc failed */
			if(new_path == NULL){
				exit(1);
			}
			strcpy(new_path, myargs[p+1]);
			strcat(new_path, "/");
			path[p] = new_path;
		}
	}

	else{
		return_val = false;
	}

	free(myargs);
	return return_val;

}


int decode_command(char *line, char **myargs){
	char * token;
	token = strtok(line, delimiters);
	myargs[0] = token;   // program name
	int myargc = 0;
	while(token != NULL) {
		myargc++;
		token = strtok(NULL, delimiters);
		/* myargs last element is equal to NULL and
		   myargc does not count the NULL element. */
		myargs[myargc] = token;
	}

	return myargc;
}


void parse_command(char *line){
	/* process redirection */
	char *output_file;
	if(false == check_valid_redirection(line, &output_file)){
		DISPLAY_ERROR
		return;
	}
	/* remove redirected files if any from the line */
	line = strtok(line, ">");

	char **myargs = malloc(255 * sizeof(char *));
	if(decode_command(line, myargs) > 0){
		run_program(myargs, output_file);
	}
	free(myargs);
}


bool check_valid_redirection(char *line, char **output_file){
	char *find_redirection = strstr(line,">");
	*output_file = NULL;
	/* if strstr finds > is will return the address of > */
	if(find_redirection)
	{
		/* ignore > */
		find_redirection++;
		*output_file = strtok(find_redirection, delimiters);
		if(*output_file == NULL){
			return false;
		}

		char *check_null = strtok(NULL, delimiters);
		/* input must have only one output file */
		if(check_null != NULL){
			return false;
		}
	}
	return true;
}


void run_program(char *myargs[], char *output_file){
	char *program_name = get_program(myargs[0]);
	if(program_name == NULL){
		DISPLAY_ERROR
		return;
	}

	/*int rc = fork();
	if (rc < 0) {
		// fork failed; exit
		exit(1);
	}
	else if (rc == 0) {
		// child (new process)*/
		if (output_file != NULL){
			/* create / open file for Redirection */
			int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
			if (fd < 0) {
				exit(1);
			}
			/* the standard output and standard error output
			   of the program should be rerouted to the file output */
			if (dup2(fd, STDOUT_FILENO) < 0 || dup2(fd, STDERR_FILENO) < 0){
				exit(1);
			}
			close(fd);
		}
		myargs[0] = program_name;   // program name
		execv(myargs[0], myargs);
		/* execv failed; exit */
		free(program_name);
		free(myargs);
		exit(1);
	/*}
	else {
		// parent goes down this path (original process)
		wait(NULL);
		free(program_name);
		free(myargs);
	}*/
}


char* get_program(char *token){
	for(int i = 0; i < num_paths; i++){
		char *program_name = (char *)malloc(strlen(path[i]) + strlen(token) + 2);
		/* malloc failed */
		if(program_name == NULL){
			exit(1);
		}
		strcpy(program_name, path[i]);
		strcat(program_name, token);
		if(0 == access(program_name, X_OK)){
			return program_name;
		}
		free(program_name);
	}
	return NULL;
}
