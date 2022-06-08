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
	int total_time;
	int remaining_time;
	int remaining_io_time;
	float prob_to_block;
	int completion_time;
	int cpu_uses;
	int cpu_time;
	int io_uses;
	int io_time;

	int time_till_block;
	int will_block;
	int newInQueue;

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
	int dispatches;
	double throughput;

} ioDevice;


Process createProcess(Process t, char *name_in, int run_time_in, float prob_to_block_in, int mode) {

  // Allocate memory for the pointers themselves and other elements
  // in the struct.
  //t = malloc(sizeof(Process));

  t.name = strdup(name_in);
  t.total_time = run_time_in;
  t.remaining_time = run_time_in;
  t.prob_to_block = prob_to_block_in;
  t.time_till_block = 0;
  t.will_block = 0;
  t.completion_time = 0;
  t.cpu_uses = 0;
  t.cpu_time = 0;
  t.io_uses = 0;
  t.io_time = 0;
  t.newInQueue = 1;
  t.remaining_io_time = 0;
  
  t.next = malloc(sizeof(Process));
  t.prev = malloc(sizeof(Process));

  return t;
}

void createCPU(CPU *t){
	t->time_busy = 0 ;
	t->time_idle = 0;
	t->dispatches = 0;
	t->utilization = 0;
	t->throughput = 0;

}
void createIO(ioDevice *t){

	t->time_busy = 0 ;
	t->time_idle = 0;
	t->dispatches = 0;
	t->utilization = 0;
	t->throughput = 0;
}

int get_count(char *file_name) {
	FILE *fp;
	int count = 0;
	char c;

	fp = fopen(file_name, "r");

	if(fp == NULL){
		fprintf(stderr, "%s", file_name);
		exit(1);
	}

	// Counting the amount of lines in the file to initialize the array
	for (c = getc(fp); c != EOF; c = getc(fp)){
       if (c == '\n') // Increment count if this character is newline
           count++;
	}
	fclose(fp);

	process_count = count;
}

	
void read_input(char *file_name, Process processes[process_count], int mode) {
	FILE *fp;
	int count_decimal;
	char *str = (char*) malloc(100 * sizeof(char));
	float prob_to_block;
	char *error_msg = (char*) malloc(100 * sizeof(char));

	fp = fopen(file_name, "r");

	for(int i = 0; i < process_count; i++){
		fgets(str, 70, fp);

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
					fprintf(stderr, "Malformed line %s(%d)\n", file_name, i+1);
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
			fprintf(stderr, "Malformed line %s(%d)\n", file_name, i+1);
			exit(1);
		}
	
		// for (int p = 0; p < 3; p++) {
		// 	printf("Input %d: %s\n", p, inputs[p]);
		// }

		// Check if process name is too long
		if(strlen(inputs[0]) > 10){
			fprintf(stderr, "name is too long, %s(%d)\n", file_name, i+1);
			exit(1);
		}

		// Check if runtime is a positive integer
		if(atoi(inputs[1]) < 1){
			fprintf(stderr, "Malformed line %s(%d)\n", file_name, i+1);
			exit(1);
		}

		// Check if probability is a number between 0 and 1
		if(atoi(inputs[2]) > 1 || atoi(inputs[2]) < 0)
		{
			fprintf(stderr, "probability < 0 or > 1 %s(%d)\n", file_name, i+1);
			exit(1);
		}

		// Populate the process struct
		processes[i] = createProcess(processes[i], inputs[0], atoi(inputs[1]), atof(inputs[2]), mode);

		// printf("%s\n", processes[i].name);
		// printf("%d\n", processes[i].remaining_time);
		// printf("%.2f\n", processes[i].prob_to_block);
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

	CPU *cpu = malloc(sizeof(CPU));
	ioDevice *io = malloc(sizeof(ioDevice));

	createIO(io);
	createCPU(cpu);

	int mode;	// Process queue mode; 0 = FCFS, 1 = RR
	int wall_clock = 0; 
	int finished_in_tick = 0;
	int quantum = 5;

	if(argc < 3) {
		fprintf(stderr, "Not enough arguments.");
		exit(1);
	}

	if(strcmp(argv[1],"-f") == 0) {
		mode = 0;
	}
	else if(strcmp(argv[1],"-r") == 0) {
		mode = 1;
	}
	else {
		fprintf(stderr, "Usage: ./prsim [-r | -f] file\n");
		exit(1);
	}

	// Seed rng with 12345
	srand(12345);

	// Get the process data and populate our struct array
	get_count(argv[2]);
	Process *processes = malloc(process_count * sizeof(Process)); // Leave space for a name to be allocated
	read_input(argv[2], processes, mode);

	printf("Processes:\n\n");
	printf("   name     CPU time  when done  cpu disp  i/o disp  i/o time\n");

	// Initialize the CPU and I/O linked list
	Process * CPU_head = &processes[0];
	Process * CPU_temp = CPU_head;
	Process * CPU_last = &processes[process_count-1];
	Process * IO_head = NULL;
	Process * IO_temp = NULL;
	Process * IO_last = NULL;
	for(int i = 1; i < process_count+1; i++)
	{
		if(i != 0) 
			CPU_temp->prev = &processes[i-1];
		if(i != process_count)
			CPU_temp->next = &processes[i];	
		else
			CPU_temp->next = NULL;


		CPU_temp = CPU_temp->next;

	}

	// Run the simulation, as long as one linked list isn't null
	
	while((CPU_head != NULL || IO_head != NULL)) {
		wall_clock++;
		//printf("\nTICK = %d ---------------------------------------\n", wall_clock);

		// CPU done routine
		if (CPU_head != NULL) {
			// Deal with a process once it's CPU or RR time is up
			if(CPU_head->remaining_time == 0 || (CPU_head->time_till_block == 0 && CPU_head->will_block)) {
				//printf("Checking end, CPU head remaining time %d \n", CPU_head->remaining_time);	
				if(CPU_head->will_block){
					// move into IO Q, special case when IO q is empty
					//printf("%s will block, moving to IO\n", CPU_head->name);
					CPU_head->will_block = 0;
					io->dispatches++;
					CPU_head->io_uses++;
					CPU_head->newInQueue = wall_clock;
					if(IO_head == NULL) {
						IO_head = CPU_head;
						IO_last = CPU_head;
						CPU_head = CPU_head->next;
						// Deal with one process left in CPU
						if (CPU_head != NULL)
							CPU_head->prev = NULL;
						else
							CPU_last = NULL;

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
							CPU_last = NULL;
						IO_last = IO_last->next;
						IO_last->next = NULL;
						// printf("\n Move into IO(1): %s \n", IO_head->name);
						// printf("Move into IO(2): %s \n", IO_last->name);	
						}
					quantum = 5;
				}
				// Deal with a complete process	
				else {
					
					if(CPU_head != NULL && CPU_head->remaining_time == 0) {
						CPU_head->completion_time = wall_clock;
						finished_in_tick = 1;

						//printf("Process %s finished at time %d\n", CPU_head->name, CPU_head->completion_time);
						printf("%-10s %6d     %6d    %6d    %6d    %6d\n", CPU_head->name, CPU_head->total_time, CPU_head->completion_time, CPU_head->cpu_uses, CPU_head->io_uses, CPU_head->io_time);

						CPU_head = CPU_head->next;
						//printf("Next Process: %s \n\n\n", CPU_head->name );
						if (CPU_head != NULL)
							CPU_head->prev = NULL;
						//printf("\nCPU_head: %s \n", CPU_head->name);
						
					}
				}

				// // Get rid of next processes that have 0 time left
				// while(CPU_head != NULL && CPU_head->remaining_time == 0) {
				// 	CPU_head->completion_time = wall_clock;
				// 	printf("Process %s finished at time %d\n", CPU_head->name, CPU_head->completion_time);
				// 	CPU_head = CPU_head->next;
				// 	//printf("Next Process: %s \n\n\n", CPU_head->name );
				// 	if (CPU_head != NULL)
				// 		CPU_head->prev = NULL;
				// 	//printf("\nCPU_head: %s \n", CPU_head->name);
				// }
			}

			if (quantum == 0 && mode == 1) {
				// Move to end of CPU queue for RR
				
				CPU_head->cpu_uses++;
				CPU_head->newInQueue = 1;
				cpu->dispatches++;
				quantum = 5;
				// Special case when one process is left in cpu
				if (CPU_head->next == NULL) {
					// Do nothing, count as a CPU dispatch?
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
		}

		// Run CPU
		if (CPU_head != NULL && finished_in_tick == 0) {
			
			// Deal with intial dispatch for each process
			if(CPU_head->remaining_time == CPU_head->total_time) {
				CPU_head->cpu_uses++;
				printf("%s dispatches: %d\n", CPU_head->name, CPU_head->cpu_uses);
				cpu->dispatches++;
			}


			// Determine if process will block, only when entering and more than 2 time units left
			if (CPU_head->newInQueue && CPU_head->remaining_time > 1){
				float ran = ((float)rand()/(float)(RAND_MAX/1));
				//printf("Random Number Being Generated: Probability of Blocking: %f \n\n", ran);
				if (ran < CPU_head->prob_to_block) { // POTENTIAL ISSUE
					CPU_head->will_block = 1;
				}
				else {
					CPU_head->will_block = 0;
				}
			}

			if(CPU_head->newInQueue && CPU_head->remaining_time < 2) {
				CPU_head->will_block = 0;
			}

			
			// printf("Will block: %d\n", will_block);
			// Run FCFS (0) CPU
			if (mode == 0) {
				// Determine time until blocking
				if(CPU_head->newInQueue){
					if (CPU_head->will_block) {
						CPU_head->time_till_block = (rand() % CPU_head->remaining_time) + 1;
						//printf("Process %s will block. Will run for %d ticks. Remaining: %d\n", CPU_head->name, CPU_head->time_till_block, CPU_head->remaining_time);

					}
					//else
						//printf("Process %s won't block. Will run for %d ticks. Remaining: %d\n", CPU_head->name, quantum, CPU_head->remaining_time);
					
					// }
					CPU_head->newInQueue = 0;
				}
				// printf("FCFS time: %d\n", remaining_CPU_time);
			}

			// Run RR (1) CPU
			else {
				if (CPU_head->newInQueue){
					if (CPU_head->will_block) {
						// Find remaining run time, must be between 1
						// and the smallest of QUANTA or remaining process runtime
						CPU_head->time_till_block = (rand() % min(CPU_head->remaining_time, quantum)) + 1;
						//printf("Process %s will block. Will run for %d ticks. Remaining: %d\n", CPU_head->name, remaining_CPU_time, CPU_head->remaining_time);
					}
					// else {
					// 	remaining_CPU_time = min(CPU_head->remaining_time, quantum);
					// 	//printf("Process %s won't block. Will run for %d ticks. Remaining: %d\n", CPU_head->name, remaining_CPU_time, CPU_head->remaining_time);
					// 	checked = 1;
					// 	//cpu->time_idle++;
					// }
					CPU_head->newInQueue = 0;
				}
				// printf("RR time: %d\n", remaining_CPU_time);
			}

			// Deal with a process that gets past checker but is finished
			if(CPU_head->remaining_time == 0) {
				CPU_head->completion_time = wall_clock;
				finished_in_tick = 1; // Maybe not?
				cpu->time_idle++;



				//printf("Process %s finished at time %d\n", CPU_head->name, CPU_head->completion_time);
				printf("%-10s %6d     %6d    %6d    %6d    %6d\n", CPU_head->name, CPU_head->total_time, CPU_head->completion_time, CPU_head->cpu_uses, CPU_head->io_uses, CPU_head->io_time);

				CPU_head = CPU_head->next;
				//printf("Next Process: %s \n\n\n", CPU_head->name );
				if (CPU_head != NULL)
					CPU_head->prev = NULL;
				//printf("\nCPU_head: %s \n", CPU_head->name);
				
			}
			else {
				//printf("Process %s about to run in CPU, with %d time left.\n", CPU_head->name, CPU_head->remaining_time);
				// if(CPU_head->remaining_time != 0) { // Don't let the time go below 0.
				// 	remaining_CPU_time--;
				// 	CPU_head->remaining_time--;	
				// 	cpu->time_busy++;
				// 	CPU_head->cpu_time++;
				// }
				
				CPU_head->remaining_time--;
				if(mode == 1) quantum--;
				cpu->time_busy++;
				if(CPU_head->will_block) CPU_head->time_till_block--;
			}

			
		}
		else {
			cpu->time_idle++;
			finished_in_tick = 0;
		}


		// I/O done routine
		if(IO_head != NULL){
			// Return to CPU
			if(IO_head->remaining_io_time == 0 && IO_head->newInQueue == 0) {
				cpu->dispatches++;
				IO_head->cpu_uses++;
				//printf("%s dispatches: %d\n", IO_head->name, IO_head->cpu_uses);
				IO_head->newInQueue = 1;
				// Place process in empty CPU Q
				if(CPU_head == NULL) {
					CPU_head = IO_head;
					CPU_last = IO_head;
					IO_head = IO_head->next;
					// Deal with special case when only one process left in IO
					if (IO_head != NULL)
						IO_head->prev = NULL;
					else 
						IO_last = NULL;
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

		// Run I/O
		if (IO_head != NULL) {
			if (IO_head->newInQueue <= wall_clock && IO_head->newInQueue != 0) { //POTENTIAL ERROR
				// Initialize the IO runtime for the process
				if(IO_head->remaining_time != 0){
					IO_head->remaining_io_time = (rand() % 30) + 1;
					//printf("Process %s will run in I/O for %d ticks\n", IO_head->name, remaining_IO_time);
					
				}
				else if(IO_head->remaining_time == 0) {
					IO_head->remaining_io_time = 1;
					
				}

				IO_head->newInQueue = 0;
			}
			if(IO_head->newInQueue == 0) {
				//printf("Process %s running in I/O, with %d time left. (Total: %d)\n", IO_head->name, IO_head->remaining_io_time, IO_head->io_time);
				IO_head->remaining_io_time--;
				IO_head->io_time++;
				io->time_busy++;

				if(IO_head->remaining_io_time == 0) {
					cpu->dispatches++;
					IO_head->cpu_uses++;
					IO_head->newInQueue = 1;
					// printf("%s dispatches: %d\n", IO_head->name, IO_head->cpu_uses);
					// Place process in empty CPU Q
					if(CPU_head == NULL) {
						CPU_head = IO_head;
						CPU_last = IO_head;
						IO_head = IO_head->next;
						// Deal with special case when only one process left in IO
						if (IO_head != NULL)
							IO_head->prev = NULL;
						else 
							IO_last = NULL;
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
				io->time_idle++;
				
			}
			
			// Don't allow a process to run in both queues in a single time unit
	
		}
		else {
			io->time_idle++;
		}
		
		// Process * temp = NULL;
		// if(CPU_head != NULL) {
		// 	temp = CPU_head;
		// 	printf("\nRemaining_time: %d CPU LIST: ", remaining_CPU_time);
		// 	while (temp != NULL){
		// 		printf("%s ", temp->name);
		// 		temp = temp->next;
		// 	}
		// }
		// if(IO_head != NULL) {
		// 	temp = IO_head;
		// 	printf("\nRemaining_IO_time: %d IO LIST: ", remaining_IO_time);
		// 	while (temp != NULL){
		// 		printf("%s ", temp->name);
		// 		temp = temp->next;
		// 	}
		// }
		// printf("\n");

	} // End simulation loop
	
	// ---------------------Input/Output---------------------

	cpu->utilization = (cpu->time_busy/(float)wall_clock);
	cpu->throughput = (process_count/(float)wall_clock);

	io->utilization = (io->time_busy/(float)wall_clock);
	io->throughput = (process_count/(float)wall_clock);


	printf("\nSystem:\n");
	printf("The wall clock time at which the simulation finished: %d\n", wall_clock);

	printf("\nCPU:\n");
	printf("Total time spent busy: %d\n", cpu->time_busy);
	printf("Total time spent idle: %d\n", cpu->time_idle);
	printf("CPU utilization: %.2f\n", cpu->utilization);
	printf("Number of dispatches: %d\n", cpu->dispatches);
	printf("Overall throughput: %.2f\n", cpu->throughput);

	printf("\nI/O device:\n");
	printf("Total time spent busy: %d\n", io->time_busy);
	printf("Total time spent idle: %d\n", io->time_idle);
	printf("I/O utilization: %.2f\n", io->utilization);
	printf("Number of dispatches: %d\n", io->dispatches);
	printf("Overall throughput: %.2f\n", io->throughput);

}
