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
	int remaining_time;
	float prob_to_block;
	int completion_time;
	int cpu_uses;
	int cpu_time;
	int io_uses;
	int io_time;

	Process *next;
	Process *prev;

};

typedef struct {
	int time_busy ;
	int time_idle;
	double utilization;
	int dispatches;
	double throughput;

} CPU;

typedef struct {
	int time_busy;
	int time_idle;
	double utilization;
	int dispatches;
	int num_jobs;
	double throughput;

} ioDevice;


Process createProcess(Process t, char *name_in, int run_time_in, float prob_to_block_in) {

  // Allocate memory for the pointers themselves and other elements
  // in the struct.
  //t = malloc(sizeof(Process));

  t.name = strdup(name_in);
  t.remaining_time = run_time_in;
  t.prob_to_block = prob_to_block_in;
  
  t.next = malloc(sizeof(Process));
  t.prev = malloc(sizeof(Process));

  return t;
}

void createCPU(CPU t){
 t.time_busy = 0 ;
 t.time_idle = 0;
 t.dispatches = 0;

}
void createIO(ioDevice t){

 t.time_busy = 0 ;
 t.time_idle = 0;
 t.dispatches = 0;
 t.num_jobs = 0;

}

int get_count(char *file_name) {
	FILE *fp;
	int count = 0;
	char c;

	fp = fopen(file_name, "r");

	if(fp == NULL){
		perror(file_name);
	}

	// Counting the amount of lines in the file to initialize the array
	for (c = getc(fp); c != EOF; c = getc(fp)){
       if (c == '\n') // Increment count if this character is newline
           count++;
	}
	fclose(fp);

	process_count = count;
}

	
void read_input(char *file_name, Process processes[process_count]) {
	FILE *fp;
	int count_decimal;
	char *str = (char*) malloc(100 * sizeof(char));
	float prob_to_block;
	char *error_msg = (char*) malloc(100 * sizeof(char));

	fp = fopen(file_name, "r");

	for(int i = 0; i < process_count; i++){
		fgets(str, 60, fp);

		 char ** inputs = (char **) calloc(3, sizeof(char*));
		 for(int l = 0; l < 3; l++)
		 {
		 	inputs[l] = (char*) calloc(30, sizeof(char));
		 }
		
		// Separate the line into the 3 inputs
		int j = 0;
		for(int k = 0; k < strlen(str); k++) {
			// Add the current character to the given input
			if(isgraph(str[k])) {
				// Check if line is malformed, with too many inputs
				if (j > 2) {
					snprintf(error_msg, 100, "%s %s%s%d%s", "Malformed line", file_name, "(", (i+1), ")");
					perror(error_msg);
					exit(1);
				}
				strncat(inputs[j], &str[k], 1);
			}
			// Increment the input on only the first whitespace after an entry
			if(k > 0 && isgraph(str[k-1]) && !(isgraph(str[k]))) {
				j++;
			}
		}

		// Check if line is malformed, with too little inputs
		if (j < 2) {
			snprintf(error_msg, 100, "%s %s%s%d%s", "Malformed line", file_name, "(", (i+1), ")");
			perror(error_msg);
			exit(1);
		}
	
		// for (int p = 0; p < 3; p++) {
		// 	printf("Input %d: %s\n", p, inputs[p]);
		// }

		// Check if process name is too long
		if(strlen(inputs[0]) > 10){
			snprintf(error_msg, 100, "%s %s%s%d%s", "name is too long", file_name, "(", (i+1), ")");
			perror(error_msg);
			exit(1);
		}

		// Check if runtime is a positive integer
		if(atoi(inputs[1]) < 1){
			snprintf(error_msg, 100, "%s %s%s%d%s", "runtime is not positive integer", file_name, "(", (i+1), ")");
			perror(error_msg);
			exit(1);
		}

		// Check if probability is a number between 0 and 1
		if(atoi(inputs[2]) > 1 || atoi(inputs[2]) < 0)
		{
			snprintf(error_msg, 100, "%s %s%s%d%s", "probability < 0 or > 1", file_name, "(", (i+1), ")");
			perror(error_msg);
			exit(1);
		}

		// Check that deciaml precision = 2
		float atofed_number = atof(inputs[2]);
		int count5 = 0;
		do{
			++count5;
			atofed_number = atofed_number * 10;
		}while((int) atofed_number % 10 != 0);
			count5--;
			
		if(count5 > 2) {
			snprintf(error_msg, 100, "%s %s%s%d%s", "probability to block is not within 2 decimal places", file_name, "(", (i+1), ")");
			perror(error_msg);
			exit(1);
		}
		
		// Populate the process struct
		processes[i] = createProcess(processes[i], inputs[0], atoi(inputs[1]), atof(inputs[2]));

		printf("%s\n", processes[i].name);
		printf("%d\n", processes[i].remaining_time);
		printf("%.2f\n", processes[i].prob_to_block);
	}
	fclose(fp);
}

int min(int a, int b) {
	if (a < b) {
		return a;
	}
	return b;
}

int main(int argc, char *argv[]) {

	CPU cpu;
	ioDevice io;

	createIO(io);
	createCPU(cpu);

	cpu.time_busy = 0;
	cpu.time_idle = 0;

	io.time_busy = 0;
	io.time_idle = 0;

	int remaining_IO_time;
	int io_checked = 0;
 	int mode;	// Process queue mode; 0 = FCFS, 1 = RR
	int wall_clock = 0;
	int will_block;
	int checked = 0;
	int remaining_CPU_time;
	const int QUANTA = 5;

	if(argc < 3) {
		perror("Not enough arguments.");
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
	get_count(argv[2]);
 	Process processes[process_count];
	read_input(argv[2], processes);

	// Initialize the CPU and I/O linked list
	Process * CPU_head = &processes[0];
	Process * CPU_temp = CPU_head;
	Process * CPU_last = NULL;
	Process * IO_head = NULL;
	Process * IO_temp = NULL;
	Process * IO_last = NULL;
	for(int i = 1; i < process_count; i++)
	{
		CPU_temp->prev = &processes[i-1];
		CPU_temp->next = &processes[i];
		
		// printf("prev: %s\n", CPU_temp->prev->name);
		// printf("next: %s\n", CPU_temp->next->name);		

		CPU_temp = CPU_temp->next;

	}
	CPU_temp->prev = &processes[process_count-2];
	CPU_last = &processes[process_count-1];
	//printf("prev: %s\n", temp->prev->name);
	//printf("CPU last: %s\n", CPU_last->name);

	// Run the simulation, as long as one linked list isn't null
	int test_count = 0;
	while((CPU_head != NULL || IO_head != NULL) && test_count < 60) {
		test_count++;

		if(CPU_head != NULL)
			printf("\nCPU_head: %s Remaining_time: %d\n", CPU_head->name, remaining_CPU_time);
		if(IO_head != NULL)
			printf("IO_head: %s Remaining_time: %d\n", IO_head->name, remaining_IO_time);

		// Run CPU
		if (CPU_head != NULL) {
			
			// Determine if process will block, only when entering and more than 2 time units left
			if (!checked){
				if (((float)rand()/(float)(RAND_MAX/1) < CPU_head->prob_to_block) && CPU_head->remaining_time > 1) {
					will_block = 1;
				}
				else {
					will_block = 0;
				}
			}
			// printf("Will block: %d\n", will_block);

			// Run FCFS (0) CPU
			if (mode == 0) {
				// Determine time until blocking
				if(!checked){
					if (will_block) {
						remaining_CPU_time = (rand() % CPU_head->remaining_time) + 1;
						checked = 1;
					}
					else {
						remaining_CPU_time = CPU_head->remaining_time;
						checked = 1;
					}
				}
				// printf("FCFS time: %d\n", remaining_CPU_time);
			}

			// Run RR (1) CPU
			else {
				if (!checked){
					if (will_block) {
						// Find remaining run time, must be between 1
						// and the smallest of QUANTA or remaining process runtime
						remaining_CPU_time = (rand() % min(CPU_head->remaining_time, QUANTA)) + 1;
						checked = 1;
					}
					else {
						remaining_CPU_time = min(CPU_head->remaining_time, QUANTA);
						checked = 1;
					}
				}
				// printf("RR time: %d\n", remaining_CPU_time);
			}

			remaining_CPU_time--;
			CPU_head->remaining_time--;	
			cpu.time_busy++;
			CPU_head->cpu_time++;

			// Deal with a process once it's CPU time is up
			if(remaining_CPU_time == 0){
				checked = 0;
				if(will_block){
					// move into IO Q, special case when IO q is empty
					io.dispatches++;
					io.num_jobs++;
					CPU_head->io_uses++;
					if(IO_head == NULL) {
						IO_head = CPU_head;
						IO_last = CPU_head;
						CPU_head = CPU_head->next;
						IO_head->next = NULL;
						IO_head->prev = NULL;
						// printf("\n Move into empty IO: %s\n", IO_head->name);
					}
					else {
						IO_last->next = CPU_head;
						CPU_head->prev = IO_last;
						CPU_head = CPU_head->next;
						// Deal with special case when only one process left in CPU (set last to NULL)
						if (CPU_head != NULL)
							CPU_head->prev = NULL;
						else
						{
							CPU_last = NULL;
							// Never occurring
							printf("\nONE LEFT IN CPU%s\n", CPU_head->name);
						} 	
					
						IO_last = IO_last->next;
						IO_last->next = NULL;

						// printf("\n Move into IO(1): %s \n", IO_head->name);
						// printf("Move into IO(2): %s \n", IO_last->name);	
						}
				}	
				else {
					
					// Check if there's still remaining time. If yes, put in back of queue. otherwise, remove it (while).
					if (CPU_head->remaining_time > 0) {
						// Special case when one process is left in cpu
						if (CPU_head->next == NULL) {
							// Do nothing	
						}
						else {
							CPU_head->prev = CPU_last;
							CPU_last->next = CPU_head;
							CPU_last = CPU_head;
							CPU_head = CPU_head->next;
							CPU_last->next = NULL;
							CPU_head->prev = NULL;
						}
					}
					else {
						// CPU_head->next->remaining_time = 0;
						// Loop until we get to a valid process to run on the CPU
						while(CPU_head != NULL && CPU_head->remaining_time == 0) {
							CPU_head = CPU_head->next;
							if (CPU_head != NULL)
								CPU_head->prev = NULL;
							//printf("\nCPU_head: %s \n", CPU_head->name);
						}
					}
				}
			}
		}
		else {
			cpu.time_idle++;
		}

		// Run I/O
		if (IO_head != NULL) {

			// Initialize the IO runtime for the process
			if(!io_checked){
				remaining_IO_time = (rand() % 30) + 1;
				io_checked = 1;
			}
			
			remaining_IO_time--;
			io.time_busy++;
			IO_head->io_time++;
			// Return process to CPU
			if(remaining_IO_time == 0) {
				io_checked = 0;
				cpu.dispatches++;
				// Place process in empty CPU Q
				if(CPU_head == NULL) {
					CPU_head = IO_head;
					CPU_last = IO_head;
					IO_head = IO_head->next;
					CPU_head->next = NULL;
					CPU_head->prev = NULL;
					//printf("IO->empty CPU head: %s \n", CPU_head->name);
				}
				else {
					CPU_last->next = IO_head;
					IO_head->prev = CPU_last;
					IO_head = IO_head->next;
					// Deal with special case when only one process left in IO
					if (IO_head != NULL)
						IO_head->prev = NULL;
					else 
						IO_last = NULL;
					CPU_last = CPU_last->next;
					CPU_last->next = NULL;
					//printf("IO->CPU last: %s \n", CPU_last->name);
				}
			}	
		}
		else{
			io.time_idle++;
		}

		wall_clock++;
	}
}