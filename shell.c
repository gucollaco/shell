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
	//printf("COMAND %s\n", *&command[9]);

	pos = get_pipe_pos(argc-1, command);
	quantity = &pos[0];

	struct commands coms[*quantity];
	//printf("quantity %d\ns", *quantity);
	//printf("value %s\n", argv[14]);

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

			int remaining = (argc-1)-used;
			printf("-----command %d-----\n", c+1);
			printf("(c/index: %d - n/beginning index: %d)\n", c, n);
			printf("(remaining arguments: %d - used/already included: %d)\n", remaining, used);
			print_command(remaining, coms[c].com);
		} else {
			// while last command still not reached
			// keep settings the commands properly, putting '|' as null
			// n value being kept because it is needed on the next iteration
			// used variable storing values to be used later (printing last command)

			coms[c].com = &command[n];
			coms[c].com[pos[c+1]-n] = NULL;
			printf("-----command %d-----\n", c+1);
			printf("(c/index: %d - n/beginning index: %d - null/nullify: %d)\n", c, n, pos[c+1]-n);
			print_command(pos[c+1]-n, coms[c].com);

			used += pos[c+1]-n;
			n = pos[c+1]+1;
		}
	}

	// creating pipe ends
	struct all_pipes pipes[*quantity-1];
	for(int i = 0; i < *quantity-1; i++) {
		if(pipe(pipes[i].pipe_ends) < 0) {
			perror("pipe");
			return -1;
		}
	}

	printf("*******************************\n");
	// redirecting the commands
	pid_t* pids = (pid_t*) malloc(*quantity * sizeof(pid_t));
	int i;
	for(i = 0; i < *quantity; i++) {
		printf("** (parent - %d) command (%d - %s)\n", getpid(), i,  coms[i].com[0]);
		//coms[i].child = fork();
        pids[i] = fork();
		if(pids[i] < 0) {
			perror("fork child");
			return -1;
		}
		if(pids[i] == 0) {
			if(i == 0) { // first command
				//close(pipes[i].pipe_ends[0]);
				//close(STDOUT_FILENO);
				printf("^^ (first child - %d) command %d\n", getpid(), i);
				close(pipes[i].pipe_ends[0]);
				dup2(pipes[i].pipe_ends[1], STDOUT_FILENO);
			}
			if(i == *quantity-1) {//*quantity-1) { // last command
				printf("^^ (last child - %d) command (last) %d\n", getpid(), i);
				//close(pipes[i-1].pipe_ends[1]);
				dup2(pipes[i-1].pipe_ends[0], STDIN_FILENO);
				close(pipes[i-1].pipe_ends[0]);
			} else { // middle command
				printf("^^ (mid child - %d) command %d\n", getpid(), i);
				//close(pipes[i-1].pipe_ends[1]);
				dup2(pipes[i-1].pipe_ends[0], STDIN_FILENO);
				close(pipes[i-1].pipe_ends[0]);
				//close(pipes[i-1].pipe_ends[0]);
				//close(pipes[i].pipe_ends[0]);
				dup2(pipes[i].pipe_ends[1], STDOUT_FILENO);
				close(pipes[i].pipe_ends[1]);
				//close(pipes[i].pipe_ends[1]);
			}

			execvp(coms[i].com[0], coms[i].com);

			/*for(int i2=0; i2 < (*quantity-1); i++) {
                close(pipes[i2].pipe_ends[0]);
                close(pipes[i2].pipe_ends[1]);
            }*/
			
			/*printf("-> filho (%d) %d\n", getpid(), i);
			if(i > 0) {
				if(dup2(all[(i-1)*2], STDIN_FILENO)) {
				
                    perror(" dup22");///j-2 0 j+1 1
                    exit(EXIT_FAILURE);
				}
			}
			if(i != *quantity-1) {
				if(dup2(all[i*2+1], STDOUT_FILENO)) {
				
                    perror(" dup223");///j-2 0 j+1 1
                    exit(EXIT_FAILURE);
				}
			}*/
			/*for(int i2=0; i2 < (*quantity-1); i++) {
                close(pipes[i2].pipe_ends[0]);
                close(pipes[i2].pipe_ends[1]);
            }*/
			
			//printf("-- filho (%d) %s %d\n", getpid(), coms[i].com[0], i);
			//execvp(coms[i].com[0], coms[i].com);
		} else {
			printf("** (parent - %d) waiting child (%d)\n", getpid(), pids[i]);
			waitpid(pids[i], NULL, 0);
		}
	}
	//for(int i = 0; i < *quantity; i++) {
	//	execvp(coms[i].com[0], coms[i].com);
	//}

	/*

	coms[0].com = &command[0];
	coms[0].com[3] = NULL;
	printf("COMAND %s\n", *&command[9]);
	coms[1].com = &command[4];
	coms[1].com[6] = NULL;
	printf("COMAND %s\n", *&command[9]);
	coms[2].com = &command[7];
	coms[2].com[9] = NULL;
	printf("COMAND %s\n", *&command[9]);
	coms[3].com = &command[10];
	coms[3].com[12] = NULL;
	printf("COMAND %s\n", *&command[9]);
	coms[4].com = &command[13];
	
	printf("-----com---\n");
	print_command(3, coms[0].com);
	printf("-----com---\n");
	print_command(2, coms[1].com);
	printf("-----com---\n");
	print_command(2, coms[2].com);
	printf("-----com---%s \n", argv[11]);
	print_command(2, coms[3].com);
	printf("-----com---\n");
	print_command(argc-1 - 15, coms[4].com);*/
	//print_command(4, 6, coms[].com);
	//print_command((argc-1)-(6+1), coms[2].com);

	//for(int i = 0; i < commands_qty; i++) {
	//	printf("-----com-----:\n");
	//	print_command(n, coms[i].com);
	//}
	//printf("-----com %d :\n", *quantity);
	//print_command(pos[1], coms[0].com);

	//for(int i = 0; i < sizeof(n)/sizeof(int); i++) {
	//	printf("eu %d", n[i]);
	//}
	return 0;

}



/*


	com1 = &command[0];
	com1[n] = NULL;
	com2 = &command[n+1];
	
	/*
	printf("-----com1:\n");
	print_command(n, com1);
	printf("-----com2:\n");
	print_command((argc-1)-(n+1), com2);
	

	int fd[2];
	if(pipe(fd) < 0) {
		perror("pipe:");
		return -1;
	}

	pid_t filho1, filho2;

	filho1 = fork();
	
	if(filho1 < 0) { //testar se fork foi bem sucedido
		perror("fork filho1:");
		return -1;
	}
	if(filho1 == 0) { //filho1 executa comando 1
		printf("-> sou filho (%d) -> pai: %d\n", getpid(), getppid());
		//close(fd[0]);
		//close(STDOUT_FILENO);
		dup2(fd[1], STDOUT_FILENO); // duplica entrada (escrita) do pipe na saída
		//close(fd[1]);
		execvp(com1[0], com1);
	} else { //pai shell aguarda filho1
		printf("-> sou pai (%d) -> filho: %d\n", getpid(), filho1);
		waitpid(filho1, NULL, 0);
		printf("-> sou pai (%d) -> filho (%d) acabou\n", getpid(), filho1);
	} //quando filho1 termina, seu resultado já está no pipe para o filho2

	
	//filho 2 vai ler do índice 0
	filho2 = fork();

	if(filho2 < 0) {
		perror("fork filho2:");
		return -1;
	}
	if(filho2 == 0) {
		printf("-> sou filho (%d) -> pai: %d\n", getpid(), getppid());
		dup2(fd[0], STDIN_FILENO);
		execvp(com2[0], com2); //entrada padrão vai ser carregada com a saída do filho 1
	} else {
		printf("-> sou pai (%d) -> filho: %d\n", getpid(), filho2);
		waitpid(filho2, NULL, 0);
		printf("-> sou pai (%d) -> filho (%d) acabou\n", getpid(), filho2);
		//printf("Pai 2\n");
	}

	return 0;*/
