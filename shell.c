#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAX 200

// struct to store the commands
typedef struct commands {
	char **com;
	pid_t child;
} commands;

// struct to store the file descriptors
typedef struct all_pipes {
   	int pipe_ends[2];
} all_pipes;

int *get_pipe_pos(int argc, char **argv) {
	// store on the array out, the indexes (of the args) in which the pipe command occurred
	// keeping the quantity of spaces between pipe commands on the cur variable 
	// later placed on the position 0 of the array
	// array space is then reallocated, for the actual quantity of occurences
	int *out = (int*) malloc(MAX * sizeof(int*));
	int cur = 1;
	for(int i = 0; i < argc; i++) {
		if(*argv[i] == '|') {
			out[cur] = i;
			cur += 1;
		}
	}
	out[0] = cur;
	int *new_out = (int*) realloc(out, cur * sizeof(int*));
	
	// debug
	/*for(int i = 1; i < cur; i++) {
		printf("indexes of occurence: %d\n", new_out[i]);
	}*/

	return out;
}

void print_command(int argc, char **com) {
	int i;
	for(i = 0; i < argc; i++) {
		printf("arg: %s\n", com[i]);
	}
}

int main(int argc, char **argv) {
	// debug - checking the sent arguments
	/*
	printf("argc = %d\n", argc);
    for(int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
	
	return 0;
	*/

	if(argc == 1) {
		printf("Uso: %s <commando1> <p> ... '|' <commando2> <p> ... \n", argv[0]);
		return 0;	
	}

	int *pos;
	int *quantity;

	char **command;
	command = &argv[1];

	pos = get_pipe_pos(argc-1, command);
	quantity = &pos[0];

	struct commands coms[*quantity];

	// separating the commands
	int n = 0;
	int used = 0;
	for(int c = 0; c < *quantity; c++) {
		// OBS: *quantity = quantity of commands, *quantity-1 = quantity of '|'
		// checking if it is the last command
		if(c == *quantity-1) {
			// when we reach the last command
			// we then need to get the total args, subtract the already 'used'
			// to get the remaining properly

			coms[c].com = &command[n];
			used += *quantity-1;

			/*int remaining = (argc-1)-used;
			printf("-----command %d-----\n", c+1);
			printf("(c/index: %d - n/beginning index: %d)\n", c, n);
			printf("(remaining arguments: %d - used/already included: %d)\n", remaining, used);
			print_command(remaining, coms[c].com);*/
		} else {
			// while last command still not reached
			// keep settings the commands properly, putting '|' as null
			// n value being kept because it is needed on the next iteration
			// used variable storing values to be used later (printing last command)

			coms[c].com = &command[n];
			coms[c].com[pos[c+1]-n] = NULL;

			/*printf("-----command %d-----\n", c+1);
			printf("(c/index: %d - n/beginning index: %d - null/nullify: %d)\n", c, n, pos[c+1]-n);
			print_command(pos[c+1]-n, coms[c].com);*/

			used += pos[c+1]-n;
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

	//printf("*******************************\n");

	// redirecting the commands
	pid_t* child = (pid_t*) malloc(*quantity * sizeof(pid_t));
	for(int i = 0; i < *quantity; i++) {
		printf("** (parent - %d) command (%d - %s)\n", getpid(), i,  coms[i].com[0]);

		for(int j=0; j<i; j++){
		    if(j!=i && j!=i-1){
		        close(pipes[j].pipe_ends[0]);
		        close(pipes[j].pipe_ends[1]);
		    }
		}
		if(*quantity > 1 && i!=0) {
			close(pipes[i-1].pipe_ends[1]);
		}
		//coms[i].child = fork();
        child[i] = fork();
		if(child[i] < 0) {
			perror("fork child");
			return -1;
		}
		if(child[i] == 0) {
			if(i == 0) { // first command
				printf("^^ (first child - %d) command %d\n", getpid(), i);
				dup2(pipes[i].pipe_ends[1], STDOUT_FILENO);
				close(pipes[i].pipe_ends[1]);
			}
			if(i == *quantity-1) {//*quantity-1) { // last command
				printf("^^ (last child - %d) command (last) %d\n", getpid(), i);
				dup2(pipes[i-1].pipe_ends[0], STDIN_FILENO);
				close(pipes[i-1].pipe_ends[0]);
			} else { // middle command
				printf("^^ (mid child - %d) command %d\n", getpid(), i);
				dup2(pipes[i-1].pipe_ends[0], STDIN_FILENO);
				close(pipes[i-1].pipe_ends[0]);
				dup2(pipes[i].pipe_ends[1], STDOUT_FILENO);
				close(pipes[i].pipe_ends[1]);
			}

			execvp(coms[i].com[0], coms[i].com);

		} else {
			printf("** (parent - %d) waiting child (%d)\n", getpid(), child[i]);
			waitpid(child[i], NULL, 0);
		}
	}

	return 0;
}
