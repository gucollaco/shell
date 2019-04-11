#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#define MAX 512

// struct to store the commands
typedef struct commands {
	char **com;
	pid_t child;
} commands;

// struct to store the file descriptors
typedef struct all_pipes {
   	int pipe_ends[2];
} all_pipes;

int *get_pipe_pos(int quantity, char **command) {
	// store on the array out, the indexes (of the args) in which the pipe command occurred
	// keeping the quantity of spaces between pipe commands on the cur variable 
	// later placed on the position 0 of the array
	int *out = malloc(MAX * sizeof(int));
	int cur = 1;
	for(int i = 0; i < quantity; i++) {
		if(*command[i] == '|') {
			out[cur] = i;
			cur += 1;
		}
	}
	out[0] = cur;
	out = realloc(out, (cur+1) * sizeof(int));

	return out;
}

char* replace_char(char* cmd, char find, char rep){
    char *cur_pos = strchr(cmd, find);
    while(cur_pos != NULL) {
        *cur_pos = rep;
        cur_pos = strchr(cur_pos, find);
    }
}

char *skip(char* s) {
	while(isspace(*s)) s++;
	return s;
}

int split(char **command) {
	printf("~ ");
	char buffer[MAX];
	fgets(buffer, MAX, stdin);

	char *cmd;
	cmd = replace_char(buffer, '\'', ' ');
	cmd = skip(buffer);
	char *next = strchr(cmd, ' ');
	int i = 0;

	while(next != NULL) {
		next[0] = '\0';
		command[i] = cmd;
		i++;
		cmd = skip(next + 1);
		next = strchr(cmd, ' ');
	}

	if(cmd[0] != '\0') {
		command[i] = cmd;
		next = strchr(cmd, '\n');
		next[0] = '\0';
		i++;
	}

	int old_i = i;
	for(int j = 0; j < i; j++) {
		if(command[j][0] == '$') {
			strcat(command[j-1], command[j]);
			command[j] = NULL;
			// shifting everyone to the left
			if(j < (i-1)) {
				for(int k = j; k < i; k++) {
					command[k] = command[k+1];
				}
			}
			i--;
		}
	}

	command[i] = NULL;
	command = realloc(command, (i+1) * sizeof(char*));

	return i;
}

int main() {
	while(1) {
		char **command = malloc(MAX * sizeof(char*));
		int *pos;
		int *quantity;
		int counter;
		
		// getting the user's input
		// then checking the pipes' position
		counter = split(command);
		pos = get_pipe_pos(counter, command);
		quantity = &pos[0];

		struct commands coms[*quantity];

		// separating the commands
		int n = 0;
		for(int c = 0; c < *quantity; c++) {
			// OBS: *quantity = quantity of commands, *quantity-1 = quantity of '|'
			// checking if it is the last command
			if(c == *quantity-1) {
				// when we reach the last command
				coms[c].com = &command[n];
			} else {
				// while last command still not reached
				// keep settings the commands properly, putting '|' as null
				// n value being kept because it is needed for the next iteration
				coms[c].com = &command[n];
				coms[c].com[pos[c+1]-n] = NULL;

				n = pos[c+1]+1;
			}
		}

		// creating pipe ends
		struct all_pipes pipes[*quantity-1];
		for(int i = 0; i < (*quantity-1); i++) {
			if(pipe(pipes[i].pipe_ends) < 0) {
				perror("pipe");
				return -1;
			}
		}

		// redirecting the commands
		pid_t child[*quantity];
		for(int i = 0; i < *quantity; i++) {
			// first, let's close what we won't need
			for(int j = 0; j < i; j++){
				if(j != i && j != i-1){
				    close(pipes[j].pipe_ends[0]);
				    close(pipes[j].pipe_ends[1]);
				}
			}
		    if(*quantity > 1 && i != 0) {
		        close(pipes[i - 1].pipe_ends[1]);
		    }

			// checking if has background sign
			// if so, the parent's work will be different
			int is_bg = 0;
			for(int k = 0; coms[i].com[k] != NULL; k++) {
				if(*coms[i].com[k] == '&') {
					is_bg = 1;
	                coms[i].com[k] = NULL;
				}
			}

			// fork being called
		    child[i] = fork();
			if(child[i] < 0) {
				perror("fork child");
				return -1;
			}
			if(child[i] == 0) { // child
				if(i == 0) { // first command
					dup2(pipes[i].pipe_ends[1], STDOUT_FILENO);
					close(pipes[i].pipe_ends[1]);
				}
				if(i == *quantity-1) { // last command
					dup2(pipes[i-1].pipe_ends[0], STDIN_FILENO);
					close(pipes[i-1].pipe_ends[0]);
				} else { // middle command
					dup2(pipes[i-1].pipe_ends[0], STDIN_FILENO);
					close(pipes[i-1].pipe_ends[0]);
					dup2(pipes[i].pipe_ends[1], STDOUT_FILENO);
					close(pipes[i].pipe_ends[1]);
				}

				// checking if < or >
				for(int k = 0; coms[i].com[k] != NULL; k++) {
					int file;
					if(*coms[i].com[k] == '<') {
		                coms[i].com[k] = NULL;
		                k++;
		                if((file = open(coms[i].com[k], O_RDONLY)) < 0) {
							perror("file open");
							return -1;
						}
		                dup2(file, STDIN_FILENO);
		                close(file);
		            } else if(*coms[i].com[k] == '>') {
		                coms[i].com[k] = NULL;
		                k++;
		                if((file = open(coms[i].com[k], O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
							perror("file open");
							return -1;
						}
		                dup2(file, STDOUT_FILENO);
		                close(file);
		            }
				}

		        if(execvp(coms[i].com[0], coms[i].com) < 0) {
		        	perror("execvp child");
		        	return -1;
		        }
			} else { // parent
				// checks if is background
				if(is_bg) {
            		printf("[%d started]\n", child[i]);
				} else {
					waitpid(child[i], NULL, 0);
				}
			}
		}
	}

	return 0;
}
