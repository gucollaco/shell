#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAX 200

// struct to store the commands
typedef struct commands {
   char **com;
} commands;

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
			/*printf("-----command %d-----\n", c+1);
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
