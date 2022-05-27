#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

// cd /mnt/c/users/benny/Desktop/Work/150_HW/HW2
int isgraph(int argument);
void srand(unsigned int seed);

int process_count;
typedef struct Process Process;

struct Process{
	char *name;
	int priority;
	int run_time;
	int remaining_time;
	float prob_to_block;
	int completion_time;
	int cpu_uses;
	int number_of_blocks;
	int io_time;

	Process *next;
	Process *prev;

};

typedef struct {
	int time_busy;
	int time_idle;
	double utilization;
	int dispatches;
	double throughput;

} CPU;

typedef struct {
	int time_busy;
	int time_idle;
	double utilization;
	int started;
	double throughput;

} ioDevice;


Process createProcess(Process t, char *name_in, int run_time_in, float prob_to_block_in) {

  // Allocate memory for the pointers themselves and other elements
  // in the struct.
  //t = malloc(sizeof(Process));

  t.name = strdup(name_in);
  t.run_time = run_time_in;
  t.prob_to_block = prob_to_block_in;
  
  t.next = malloc(sizeof(Process));
  t.prev = malloc(sizeof(Process));

  return t;
}

int get_count(char *file_name) {
	FILE *fp;
	int count = 0;
	char c;

	fp = fopen(file_name, "r");

	if(fp == NULL){
		perror("File cannot be opened \n");
	}

	// Counting the amount of lines in the file to initialize the array
	for (c = getc(fp); c != EOF; c = getc(fp)){
       if (c == '\n') // Increment count if this character is newline
           count = count + 1;
	}
	fclose(fp);

	process_count = count;
}

	
void read_input(char *file_name, Process processes[process_count]) {
	FILE *fp;
	int count_decimal;
	char *str = (char*) malloc(61 * sizeof(char));
	float prob_to_block;

	fp = fopen(file_name, "r");

	if(fp == NULL){
		perror("File cannot be opened \n");
	}

	for(int i = 0; i < process_count; i++){
		fgets(str, 60, fp);

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
	
		// for (int p = 0; p < 3; p++) {
		// 	printf("Input %d: %s\n", p, inputs[p]);
		// }

		
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
		processes[i] = createProcess(processes[i], inputs[0], atoi(inputs[1]), atof(inputs[2]));

		printf("%s\n", processes[i].name);
		printf("%d\n", processes[i].run_time);
		printf("%.2f\n", processes[i].prob_to_block);
	}
	fclose(fp);
}

int main(int argc, char *argv[]) {

 	int mode;	// Process queue mode; 0 = FCFS, 1 = RR
	int wall_clock = 0;
	int will_block;

	if(argc < 2) {
		perror("No flag entered.");
		exit(1);
	}

	if(strcmp(argv[1],"-f") == 0) {
		mode = 0;
	}
	else if(strcmp(argv[1],"-r") == 0) {
		mode = 1;
	}
	else {
		perror("Invalid flag entered.");
		exit(1);
	}

	// Seed rng with 12345
	srand(12345);

	// Get the process data and populate our struct array
	get_count("input.txt");
 	Process processes[process_count];
	read_input("input.txt", processes);

	// Initialize the CPU and I/O linked list
	Process * CPU_head = &processes[0];
	Process * IO_head = NULL;
	Process * CPU_temp = CPU_head;
	Process * IO_temp = NULL;
	for(int i = 1; i < process_count; i++)
	{
		CPU_temp->prev = &processes[i-1];
		CPU_temp->next = &processes[i];
		
		// printf("prev: %s\n", CPU_temp->prev->name);
		// printf("next: %s\n", CPU_temp->next->name);		

		CPU_temp = CPU_temp->next;

	}
	CPU_temp->prev = &processes[process_count-2];
	//printf("prev: %s\n", temp->prev->name);
	

	// Run the simulation, as long as one linked list isn't null
	if(CPU_head != NULL || IO_head != NULL) {

		// Run CPU
		if (CPU_head != NULL) {
			// Determine if process will block
			if ((float)rand()/(float)(RAND_MAX/1)< CPU_head->prob_to_block) {
				will_block = 1;
			}
			else {
				will_block = 0;
			}

			// Run FCFS CPU or RR CPU
			if (mode == 0) {

			}
			else {

			}

		}

		// Run I/O
		if (IO_head != NULL) {

		}

	}

}