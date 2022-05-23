#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

// cd /mnt/c/users/benny/Desktop/Work/150_HW/HW2
int isgraph(int argument);

struct Process {
	char name[10];
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
		// printf("%s", str);

		 char ** inputs = (char **) calloc(3, sizeof(char*));
		 for(int l = 0; l < 3; l++)
		 {
		 	inputs[l] = (char*) calloc(20, sizeof(char));
		 }
		
		// Separate the line into the 3 inputs
		int j = 0;
		for(int k = 0; k < strlen(str); k++) {
			// Add the current character to the given input
			if(isgraph(str[k])) {
				strncat(inputs[j], &str[k], 1);
			}
			// Increment the input on only the first whitespace after an entry
			if(k > 0 && isgraph(str[k-1]) && !(isgraph(str[k]))) {
				j = j + 1;
			}
		}
	
		for (int p = 0; p < 3; p++) {
			printf("Input %d: %s\n", p, inputs[p]);
		}

		
		if(strlen(inputs[0]) > 10){
			perror("process name is too large");
			exit(1);
		}

		if(atoi(inputs[1]) < 1){
			perror("runtime is not 1 or greater");
			exit(1);
		}

		if(atoi(inputs[2]) > 1 || atoi(inputs[2]) < 0)
		{
			perror("the probability to block is not between 0 and 1");
			exit(1);
		}

		
		/*prob_to_block = abs(atof(inputs[2]));
		while(prob_to_block != 0){
			prob_to_block = prob_to_block * 10;
			count_decimal = count_decimal + 1;
			prob_to_block = prob_to_block - prob_to_block;
		}*/

		float atoied_number = atof(inputs[2]);
		int count5 = 0;
		do{
			++count5;
			atoied_number = atoied_number * 10;
		}while((int) atoied_number % 10 != 0);
			count5 = count5 - 1;
			

		if(count5 > 2)
		{
			perror("the probability to block is not within 2 decimal places");
			exit(1);
		}
		
		// Populate the process struct
		printf("%s", inputs[0]);
		//processes[i].name = inputs[0];
		processes[i].run_time = atoi(inputs[1]);
		processes[i].prob_to_block = atof(inputs[2]);

	}

}

int main(int argc, char *argv[]) {
 
	read_input("input.txt");

}