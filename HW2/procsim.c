#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

// cd /mnt/c/users/benny/Desktop/Work/150_HW/HW2
int isgraph(int argument);

struct Process {
	//char *name = (char *) malloc(sizeof(char) * 11);
	int priority;
	int run_time;
	int remaining_time;
	float prob_to_block;
	int completion_time;
	int cpu_uses;
	int number_of_blocks;
	int io_time;


	struct Process *next;
	struct Process *prev;
};



struct Process * read_input(char *file_name) {
	FILE *fp;
	int count = 0;
	int count_decimal;
	char c;
	char *str = (char*) malloc(61 * sizeof(char));
	//char *newString = (char*) malloc(61 * sizeof(char));
	char *process_Name = (char*) malloc(11 * sizeof(char));;
	int runtime;
	float prob_to_block;

	fp = fopen(file_name, "r");

	if(fp == NULL){
		perror("File cannot be opened \n");
	}

	// Counting the amount of lines in the file to initialize the array
	for (c = getc(fp); c != EOF; c = getc(fp)){
       if (c == '\n') // Increment count if this character is newline
           count = count + 1;
	}

	rewind(fp);

	struct Process processes[count];

	for(int i = 0; i < count; i++){
		fgets(str, 60, fp);
		printf("%s", str);

		// Separate the line into the 3 inputs
		 char ** inputs = (char **) calloc(2, sizeof(char*));
		 for(int l = 0; l < 2; l++)
		 {
		 	inputs[l] = (char*) calloc(20, sizeof(char));
		 }



		int j = 0;
		for(int k = 0; k < strlen(str); k++) {
			if(isgraph(str[k])) {
				strncat(inputs[j], &str[k], 1);
			}
			if(isgraph(str[k-1])  && isgraph(str[k]) ) {
				j = j + 1;
			}
		}

		for (int p = 0; p < 3; p++) {
			printf("Input %d: %s", p, inputs[p]);
		}


		/*
		newString = strtok(str, " ");
		process_Name = newString;
		printf("%s is the process name", process_Name);

		newString = strtok(str, " ");
		runtime = atoi(newString);
		printf("%d is the runtime", runtime);

		newString = strtok(str, " ");
		prob_to_block = atof(newString);
		printf("%f is the percentage\n", prob_to_block);
		*/

		/*
		if(strlen(process_Name) > 10){
			perror("process name is too large");
		}

		if(runtime < 1){
			perror("runtime is not 1 or greater");
		}


		prob_to_block = abs(prob_to_block);
		prob_to_block = prob_to_block - prob_to_block;
		while(prob_to_block != 0){
			prob_to_block = prob_to_block * 10;
			count_decimal = count_decimal + 1;
			prob_to_block = prob_to_block - prob_to_block;
		}

		if(count_decimal != 2)
		{
			perror("the probability to block is not within 2 decimal places");
		}

		if(prob_to_block > 1 && prob_to_block < 0)
		{
			perror("the probability to block is not between 0 and 1");
		}
		*/
	}

}

int main(int argc, char *argv[]) {
 
	read_input("input.txt");

}