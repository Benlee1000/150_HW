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
	int run_time;
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
	int num_jobs;
	double throughput;

} ioDevice;


Process createProcess(Process t, char *name_in, int run_time_in, float prob_to_block_in, int mode) {

  // Allocate memory for the pointers themselves and other elements
  // in the struct.
  //t = malloc(sizeof(Process));

  t.name = strdup(name_in);
  t.run_time = run_time_in;
  t.remaining_time = run_time_in;
  t.prob_to_block = prob_to_block_in;

  t.completion_time = 0;
  if (mode == 1)
  	t.cpu_uses = 1;
  else
  	t.cpu_uses = 0;
  t.cpu_time = 0;
  t.io_uses = 0;
  t.io_time = 0;
  
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
	t->num_jobs = 0;
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
		/*
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
		*/

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
	int remaining_CPU_time = 0;
	int remaining_IO_time = 0;
	int checked = 0;
	int io_checked = 0;
	int wall_clock = 0; 
	int will_block;
	int just_arrived = 0;
	
	const int QUANTA = 5;

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
	
	//printf("prev: %s\n", temp->prev->name);
	//printf("CPU last: %s\n", CPU_last->name);

	// Process * temp = NULL;
	// temp = CPU_head;
	// printf("\nCPU LIST: ");
	// while (temp != NULL) {
	// 	printf("%s ", temp->name);
	// 	temp = temp->next;
	// }


	// Run the simulation, as long as one linked list isn't null
	
	while((CPU_head != NULL || IO_head != NULL)) {
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
		//printf("\n");
		
		printf("\nTICK = %d ---------------------------------------\n", wall_clock);

		// Run CPU
		if (CPU_head != NULL && wall_clock > 0) {
			if (wall_clock == 1) {
				wall_clock++;
			}

			// Determine if process will block, only when entering and more than 2 time units left
			if (!checked && CPU_head->remaining_time > 1){
				float ran = ((float)rand()/(float)(RAND_MAX/1));
				printf("Random Number Being Generated: Probability of Blocking: %f \n\n", ran);
				if (CPU_head->remaining_time > 1 && ran < CPU_head->prob_to_block) { // POTENTIAL ISSUE
					will_block = 1;
				}
				else {
					will_block = 0;
				}
			}

			if(!checked && CPU_head->remaining_time < 2) {
				will_block = 0;
			}

			if(!checked) {
				CPU_head->cpu_uses++;
				cpu->dispatches++;
				printf("Count CPU DISPATCH: %s(%d)\n", CPU_head->name, CPU_head->cpu_uses);
			}

			// printf("Will block: %d\n", will_block);
			// Run FCFS (0) CPU
			if (mode == 0) {
				// Determine time until blocking
				if(!checked){
					if (will_block) {
						remaining_CPU_time = (rand() % CPU_head->remaining_time) + 1;
						printf("Process %s will block. Will run for %d ticks. Remaining: %d\n", CPU_head->name, remaining_CPU_time, CPU_head->remaining_time);
						checked = 1;
						//cpu->time_idle++;

					}
					else {
						remaining_CPU_time = CPU_head->remaining_time;
						printf("Process %s won't block. Will run for %d ticks. Remaining: %d\n", CPU_head->name, remaining_CPU_time, CPU_head->remaining_time);
						checked = 1;
						//if (CPU_head->remaining_time != 0)
							//cpu->time_idle++;
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
						printf("Process %s will block. Will run for %d ticks. Remaining: %d\n", CPU_head->name, remaining_CPU_time, CPU_head->remaining_time);
						checked = 1;
						//cpu->time_idle++;
					}
					else {
						remaining_CPU_time = min(CPU_head->remaining_time, QUANTA);
						printf("Process %s won't block. Will run for %d ticks. Remaining: %d\n", CPU_head->name, remaining_CPU_time, CPU_head->remaining_time);
						checked = 1;
						//cpu->time_idle++;
					}
				}
				// printf("RR time: %d\n", remaining_CPU_time);
			}

			// Need to add CPU dipatch when process first comes in

			// Count a CPU dispatch when a process is first loaded
			// if (CPU_head->run_time == CPU_head->remaining_time && !retrieving_process) {
			// 	CPU_head->cpu_uses++;
			// 	cpu->dispatches++;	
			// }


			// if(retrieving_process) {
			// 	printf("Placing process %s on the CPU\n", CPU_head->name);
			// 	// if(CPU_head != NULL && CPU_head->remaining_time == 0) {
			// 	// 	checked = 0;
			// 	// 	CPU_head->completion_time = wall_clock;
			// 	// 	printf("Proces %s finished at time %d\n", CPU_head->name, CPU_head->completion_time);
			// 	// 	CPU_head = CPU_head->next;
			// 	// 	if (CPU_head != NULL)
			// 	// 		CPU_head->prev = NULL;
			// 	// 	//printf("\nCPU_head: %s \n", CPU_head->name);
			// 	// }
			// } 

			printf("Process %s running in CPU, with %d time left.\n", CPU_head->name, remaining_CPU_time);
			if(CPU_head->remaining_time != 0) { // Don't let the time go below 0.
				remaining_CPU_time--;
				CPU_head->remaining_time--;	
				cpu->time_busy++;
				CPU_head->cpu_time++;
			}
			
			// Deal with a process once it's CPU time is up
			if(remaining_CPU_time == 0){
				printf("Checking end of process\n");
				printf("CPU head remaining time %d \n", CPU_head->remaining_time);	
				checked = 0;
				if(will_block){
					will_block = 0;
					// move into IO Q, special case when IO q is empty
					io->dispatches++;
					io->num_jobs++;
					CPU_head->io_uses++;
					if(IO_head == NULL) {
						just_arrived = 1; // Don't allow CPU and I/O runtime for the same process in same time period
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
				}	
				else {
					printf("CPU head remaining time %d \n\n", CPU_head->remaining_time);
					// Check if there's still remaining time. If yes, put in back of queue. otherwise, remove it (while).
					if (CPU_head->remaining_time > 0) {
						//CPU_head->cpu_uses++;
						//cpu->dispatches++;
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
					else {
				
						if(CPU_head != NULL && CPU_head->remaining_time == 0) {
							CPU_head->completion_time = wall_clock;
							printf("Process %s finished at time %d\n", CPU_head->name, CPU_head->completion_time);
							CPU_head = CPU_head->next;
							//printf("Next Process: %s \n\n\n", CPU_head->name );
							if (CPU_head != NULL)
								CPU_head->prev = NULL;
							//printf("\nCPU_head: %s \n", CPU_head->name);
						}
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
		}
		else {
			cpu->time_idle++;
		}

		// Run I/O
		if (IO_head != NULL && wall_clock > 0) {
			if (!just_arrived) {
				// Initialize the IO runtime for the process
				if(!io_checked && IO_head->remaining_time != 0){
					remaining_IO_time = (rand() % 30) + 1;
					printf("Process %s will run in I/O for %d ticks\n", IO_head->name, remaining_IO_time);
					io_checked = 1;
				}
				else if(!io_checked && IO_head->remaining_time == 0) {
					remaining_IO_time = 1;
					io_checked = 1;
				}

				remaining_IO_time--;
				io->time_busy++;
				IO_head->io_time++;

				printf("Process %s running in I/O, with %d time left. (Total: %d)\n", IO_head->name, remaining_IO_time, IO_head->io_time);
			}
			// Don't allow a process to run in both queues in a single time unit
			else {

				printf("Transferring %s to the IO q. (Will not run b/c same tick).\n", IO_head->name);
				io->time_idle++;
				//Potentially something we should remove ^^^
				just_arrived = 0;
			}
			
			// Return process to CPU
			if(remaining_IO_time == 0 && io_checked) {
				io_checked = 0;
				//cpu->dispatches++;
				//IO_head->cpu_uses++;
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

		wall_clock++;
	} // End simulation loop
	
	// ---------------------Input/Output---------------------

	wall_clock--; // Wall clock increased one too many times, when last process finishes


	if (mode == 1)
		cpu->dispatches = cpu->dispatches + process_count;
	else {
		processes[process_count-1].completion_time++;
		wall_clock++;
	 	cpu->time_idle = cpu->time_idle + process_count - 1;
	 	io->time_idle++;
	}

	cpu->utilization = (cpu->time_busy/(float)wall_clock);
	cpu->throughput = (process_count/(float)wall_clock);

	io->utilization = (io->time_busy/(float)wall_clock);
	io->throughput = (process_count/(float)wall_clock);
	

	//printf("First time: %d\n", processes[0].cpu_time);

	printf("Processes:\n");
	printf("name\t\tCPU time\twhen done\tcpu disp\ti/o disp\ti/o time\n");
	for(int i = 0; i < process_count; i++)
	{
	    
	    printf("%-10s \t%d \t\t%d \t\t%d \t\t%d \t\t%d\n", processes[i].name, processes[i].cpu_time, processes[i].completion_time, processes[i].cpu_uses, processes[i].io_uses, processes[i].io_time);
	}


	printf("\nSystem: \n");
	printf("The wall clock time at which the simulation finished: %d\n", wall_clock);

	printf("\nCPU: \n");
	printf("Total time spent busy: %d \n", cpu->time_busy);
	printf("Total time spent idle: %d \n", cpu->time_idle);
	printf("CPU utilization: %.2f \n", cpu->utilization);
	printf("Number of dispatches: %d \n", cpu->dispatches);
	printf("Overall throughpout: %.2f \n", cpu->throughput);

	printf("\nI/O device: \n");
	printf("Total time spent busy: %d \n", io->time_busy);
	printf("Total time spent idle: %d \n", io->time_idle);
	printf("I/O utilization: %.2f \n", io->utilization);
	printf("Number of dispatches: %d \n", io->dispatches);
	printf("Overall throughput: %.2f \n", io->throughput);


}
