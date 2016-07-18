/**
 * @file scheduler.c
 * @author Adrian Rasmussen
 *
 * @brief Operating Systems 200 - Simulates CPU and I/O scheduler
 *		  Usage: scheduler jobFile
 *		  Error handling causes code to look messy, due to arbitrary restrictions
 *		  on multiple returns and use of GOTO, which would be much cleaner for exceptions
 * @version 18/05/2013
 * @since 05/05/2013
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "scheduler.h"
#include "pa.h"
#include "linkedList.h"
#include "error.h"

static int errState = 0; /**Indicates if there is an error in cpu or I/O thread*/

static int cpuTime = 0; /**Total time spent in CPU*/
static int cpuIdleTime = 0; /**Time CPU is idle*/
static int cpuWaitingTime = 0; /**Total time all processes wait for CPU*/

static int ioTime = 0; /**Total time spent in I/O*/
static int ioIdleTime = 0; /**Time I/O is idle*/
static int ioWaitingTime = 0; /**Total time processes spend waiting for I/O*/

static LinkedList *cpuQueue = NULL;
static LinkedList *ioQueue = NULL; /**Linked lists in FIFO order to store processes needing CPU or I/O*/

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /**Mutex to lock and unlock*/
static pthread_cond_t cpuEmpty = PTHREAD_COND_INITIALIZER; /**Condition to signal when cpuQueue has a process added*/
static pthread_cond_t ioEmpty = PTHREAD_COND_INITIALIZER; /**Condition to signal when ioQueue has a process added*/

int main( int argc, char* argv[])
{
	ERROR_CODE code;
	int numProcesses,status = EXIT_FAILURE;
	char **pidArray;

	if ( argc == 2)
	{
		/*Get list of process file names in array*/
		code = getLines(argv[1], &pidArray, &numProcesses);
		if (code == E_SUCCESS)
		{
			/*Run scheduler process*/
			code = scheduler(pidArray, numProcesses); 
			if ( code == E_SUCCESS)
			{
				status = EXIT_SUCCESS;
			}
			else
			{
				printf("Scheduler failed, %s\n", getErrorName(code));
			}
		}
		else
		{
			printf("Could not get file names from %s, because %s\n", argv[1], getErrorName(code));
		}
	}
	else
	{
		printf("Invalid number of parameters. Usage: scheduler file\n");		
	}

	return status;
}

/**
 * Method:    scheduler
 * @brief     Simulates a CPU {@link cpu()} and I/O {@link io()} scheduler, reading list of process from file
 *			  and adding them to a queue; Prints PAs to file and calculates utilization and waiting times.
 * @param 	  processes - array of process file names, PID-#, which each contain list of activities
 * @param 	  numProcesses - number of total processes
 * @return    ERROR_CODE enum value: E_SUCCESS if successful
 *									 E_FILE_NOT_FOUND if log file couldn't be opened
 *									 E_OUT_OF_MEMORY if malloc failed
 **/
ERROR_CODE scheduler(char** processes, int numProcesses)
{
	int i;
	FILE *output;
	Pa *process;
	ERROR_CODE result, returnValCPU, returnValIO;
	pthread_t cpuThread, ioThread;

	/*Create and allocate cpu and io queues*/
	newLinkedList(&cpuQueue);
	newLinkedList(&ioQueue);
	if (cpuQueue && ioQueue)
	{
		/*Open log file to print new processes to*/
		output = fopen("log-A", "a");

		if (output)
		{

			i = 0;
			result = E_SUCCESS;
			/*Create PA for each process, add to CPU queue*/
			while( i < numProcesses && result == E_SUCCESS)
			{
				result = createPA(&process, processes[i]);
				if (result == E_SUCCESS)
				{
					free(processes[i]);
					processes[i] = NULL;

					/*Add process to end of cpuQueue*/
					result = enqueue(cpuQueue, process);

					/*Add new process to log file*/
					if ( result == E_SUCCESS)
					{
						fprintf(output, "New process:\n");
						fprintf(output, "PID=%d\n", process->pid);
						fprintf(output, "AC=%d\n", process->ac);
						fprintf(output, "State=CPU\n");
						fprintf(output, "Arrive=%d\n", process->arriveTime);
						fprintf(output, "Time=%d\n\n", process->remainingTime);
					}
				}
				
				/*createPa may succeed, but enqueue could fail so must be separate if check
				 *rather than else*/
				if ( result != E_SUCCESS)
				{
					printf("Could not create PA for process %d (numbered in job list order), %s\n", i+1, getErrorName(result));
					/*Free remaining processes*/
					for ( i = i; i < numProcesses; i++)
					{
						free(processes[i]);
						processes[i] = NULL;
					}
				}

				i++;
			}
			
			fclose(output);
			free(processes);
			processes = NULL;

			if (result == E_SUCCESS)
			{
				/*Create two threads, one for cpu() and one for io()*/
				pthread_create(&cpuThread, NULL, &cpu, NULL);
				pthread_create(&ioThread, NULL, &io, NULL);
	
				pthread_join(cpuThread, (void**) &returnValCPU);
				pthread_join(ioThread, (void**) &returnValIO);
	
				deleteLinkedList(cpuQueue);
				deleteLinkedList(ioQueue);

				/*Check return values of CPU and IO threads*/
				if(returnValCPU != E_SUCCESS)
				{
					result = returnValCPU;
				}
				else if(returnValIO != E_SUCCESS)
				{
					result = returnValIO;
				}
				else /*Everything executed successfully, output log file*/
				{
					output = fopen("log-B", "a");
					if ( output)
					{		
						/*Calculate statistics about CPU and I/O utilization and average waiting time, print to file*/
						fprintf(output, "Average waiting time in CPU queue: %.2f\n", (double) cpuWaitingTime / (double) numProcesses);
						fprintf(output, "Average waiting time in I/O queue: %.2f\n", (double) ioWaitingTime / (double) numProcesses);
						fprintf(output, "CPU utilization: %.2f%%\n", (double) 100 * (cpuTime - cpuIdleTime) / (double) cpuTime);
						fprintf(output, "I/O utilization: %.2f%%\n\n", (double) 100 * (ioTime - ioIdleTime) / (double) ioTime);
			
						fclose(output);
					}
					else
					{
						perror("log-B");
						result = E_FILE_NOT_FOUND;
					}
				}
			}
		}
		else
		{
			perror("log-A");
			result = E_FILE_NOT_FOUND;
		}
	}
	else
	{
		/*Free any allocated memory, delete function handles NULL so this is safe*/
		deleteLinkedList(cpuQueue);
		deleteLinkedList(ioQueue);
		result = E_OUT_OF_MEMORY;
	}
	
	return result;
}

/**
 * Method:    cpu
 * @brief     Simulates execution of processes that need CPU, running a process
 *			  for the time it needs and then updating it's PA, then adding it to the I/O queue
 * @return    (void*) ERROR_CODE enum value: E_SUCCESS if successful
 *											 E_FILE_NOT_FOUND if file couldn't be opened			  
 **/
void* cpu( void* arg)
{
	int i;
	Pa *process;
	FILE *output;
	
	for (;;)
	{
		/*dequeue(cpu) ... enqueue(io) must be mutually exclusive*/ 
		pthread_mutex_lock( &mutex);	

		while (isEmpty(cpuQueue) && errState == 0)
		{
			pthread_cond_wait( &cpuEmpty, &mutex);
		}	

		/*Check state of other thread - if 0, continue, 1 exit successfully, 2 exit failure*/
		if ( errState == 1)
		{
			return (void*) E_SUCCESS;
		}
		else if (errState == 2)
		{
			return (void*) E_FAILURE;
		}

		/*Get next process from queue*/
		assert(cpuQueue != NULL && cpuQueue->head != NULL);
		dequeue( cpuQueue, &process);
	
		/*Update cpuTime to correct value, due to time spent idle*/
		if ( cpuTime < process->arriveTime)
		{
			cpuIdleTime += process->arriveTime - cpuTime;
			cpuTime = process->arriveTime;
		}
	
		/*Add waiting time to running total, result may be 0*/
		cpuWaitingTime += cpuTime - process->arriveTime;
		
		/*Execute process by incrementing cpuTime*/
		for ( i = 0; i < process->remainingTime; i++)
		{
			cpuTime++;
		}
	
		/*If process still requires I/O*/
		if( process->ac < process->totalAc)
		{
			process->ac++;
			process->state = 1;
			/*Arrive time in I/O is current arrive time + time waiting in CPU + burst time*/
			process->arriveTime = cpuTime;
			process->remainingTime = process->times[process->ac-1];
			
			output = fopen("log-A", "a");
			if ( output == NULL)
			{
				perror("log-A");
				
				free(process->times);
				free(process);
				process = NULL;
				
				errState = 2;
				pthread_cond_signal(&ioEmpty);		
				pthread_mutex_unlock(&mutex);
				return (void*) E_FILE_NOT_FOUND;
			}
			
			/*Print process' activity to file*/
			fprintf(output, "Finishing CPU activity:\n");
			fprintf(output, "PID=%d\n", process->pid);
			fprintf(output, "AC=%d\n", process->ac);
			fprintf(output, "State=I/O\n");
			fprintf(output, "Arrive=%d\n", process->arriveTime);
			fprintf(output, "Time=%d\n\n", process->remainingTime);

			fclose(output);

			/*Add updated process to the end of the ioQueue*/
			enqueue( ioQueue, process);
		}
		else /*Process terminated*/
		{
			output = fopen("log-B", "a");
			if ( output == NULL) /*Terminate thread, signal ioThread to terminate too*/
			{		
				perror("log-B");
				
				/*Free memory allocated to process*/
				free(process->times);
				free(process);
				process = NULL;

				errState = 2;
				pthread_cond_signal(&ioEmpty);		
				pthread_mutex_unlock(&mutex);
				return (void*) E_FILE_NOT_FOUND;
			}
			fprintf(output, "Process PID-%d is terminated.\n", process->pid);
			fprintf(output, "PID=%d\n", process->pid);
			fprintf(output, "AC=%d\n", process->ac);
			fprintf(output, "State=CPU\n");
			fprintf(output, "Arrive=%d\n", process->arriveTime);
			fprintf(output, "Time=%d\n\n", process->remainingTime);
	
			fclose(output);
	
			/*Free memory allocated to process*/
			free(process->times);
			free(process);
			process = NULL;

			/*Must only check either before io calls dequeue(io) or after enqueue(cpu)
			*as otherwise both may temporarily be empty so this would exit*/

			/*Terminated a process, so if both queues are now empty then finish*/
			if( isEmpty(cpuQueue) && isEmpty(ioQueue))
			{
				/*Unblock ioThread so it can exit gracefully*/
				errState = 1;
				pthread_cond_signal(&ioEmpty);		
				pthread_mutex_unlock(&mutex);
				
				return (void*) E_SUCCESS;
			}
		}
		/*Added process to ioQueue, signal the thread and unblock*/
		pthread_cond_signal(&ioEmpty);
		pthread_mutex_unlock(&mutex);
	}
}

/**
 * Method:    io
 * @brief     Simulates execution of processes that need I/O, running a process
 *			  for the time it needs and then updating it's PA, then adding it to the CPU queue
 * @return    (void*) ERROR_CODE enum value: E_SUCCESS if successful
 *											 E_FILE_NOT_FOUND if file couldn't be opened
 **/
void* io( void* arg)
{
	int i;
	Pa *process;
	FILE *output;

	for(;;)
	{
		pthread_mutex_lock( &mutex);		
		
		/*Exit loop is ioQueue has a process added or other thread has encountered error*/
		while( isEmpty(ioQueue) && errState == 0)
		{
			pthread_cond_wait(&ioEmpty, &mutex);
		}

		/*Check state of other thread*/
		if ( errState == 1)
		{
			return (void*) E_SUCCESS;
		}
		else if (errState == 2)
		{
			return (void*) E_FAILURE;
		}


		dequeue( ioQueue, &process);

		/*Update actual ioTime if previously idle*/
		if ( ioTime < process->arriveTime)
		{
			ioIdleTime += process->arriveTime - ioTime;
			ioTime = process->arriveTime;
		}
		ioWaitingTime += ioTime - process->arriveTime;
		for ( i = 0; i < process->remainingTime; i++)
		{
			/*Executing Process*/
			ioTime++;
		}

		process->ac++;
		process->state = 1;
		process->arriveTime = ioTime;
		process->remainingTime = process->times[process->ac-1];
		output = fopen("log-A", "a");
		if ( output == NULL)
		{			
			perror("log-A");
			
			/*Free memory allocated to process as it won't be added to queue*/
			free(process->times);
			free(process);
			process = NULL;

			errState = 2;
			pthread_cond_signal(&cpuEmpty);
			pthread_mutex_unlock(&mutex);

			return (void*) E_FILE_NOT_FOUND;
		}
		fprintf(output, "Finishing I/O activity:\n");
		fprintf(output, "PID=%d\n", process->pid);
		fprintf(output, "AC=%d\n", process->ac);
		fprintf(output, "State=CPU\n");
		fprintf(output, "Arrive=%d\n", process->arriveTime);
		fprintf(output, "Time=%d\n\n", process->remainingTime);

		fclose(output);

		enqueue( cpuQueue, process);

		pthread_cond_signal(&cpuEmpty);
		pthread_mutex_unlock(&mutex);
	}
}

/**
 * Method:    getLines
 * @brief     Extracts the lines from a file and stores each line as an element in an array
 * @param 	  job - name of file
 * @param 	  lines - address to save lines array to
 * @param 	  numJobs - address to save number of lines to
 * @return    ERROR_CODE enum value: E_SUCCESS if successful
 *									 E_FILE_NOT_FOUND if file couldn't be opened
 *									 E_OUT_OF_MEMORY if malloc unsuccessful
 **/
ERROR_CODE getLines( char *job, char ***lines, int *numJobs )
{
	int j, numChars;
	int i, numLines;
	char *buffer;
	long bytes;
	FILE *f;

	/*TODO: change to just r when using in linux*/
	f = fopen(job, "r");
	if ( f == NULL)
	{
		perror(job);
		return E_FILE_NOT_FOUND;
	}
	/*Calculate size of file*/
	fseek( f, 0, SEEK_END);
	bytes = ftell(f);
	rewind(f);
	
	/*Allocate buffer as file size +1 for terminating \0*/
	buffer = (char*) malloc( bytes + 1);
	if ( buffer == NULL)
	{
		printf("Could not allocate memory for buffer\n");
		fclose(f);
		return E_OUT_OF_MEMORY;
	}
	
	/*Read file into buffer*/
	fread(buffer, bytes, 1, f);
	fclose(f);

	/*Count num lines in buffer, change \n to \0*/
	numLines = 0;
	for ( i = 0; i < bytes; i++)
	{
		if ( buffer[i] == '\n')
		{
			buffer[i] = '\0';
			numLines++;
		}
		else if ( i == bytes - 1) /*last char not new line*/
		{
			numLines++;
		}
	}

	*numJobs = numLines;
	buffer[bytes] = '\0';

	(*lines) = (char **) malloc( numLines * sizeof *(*lines));
	if ( (*lines) == NULL)
	{
		printf("Could not allocate memory for pidArray\n");
		free(buffer);
		buffer = NULL;
		return E_OUT_OF_MEMORY;
	}

	/*Count num chars in each line, malloc array for each line and copy into array*/
	j = 0;
	numChars = 1;
	for ( i = 0; i <= bytes; i++)
	{
		if ( buffer[i] == '\0' && j < numLines)
		{
			(*lines)[j] = (char*) malloc (numChars);
			if ( (*lines)[j] != NULL)
			{
				strncpy((*lines)[j], &(buffer[i+1-numChars]), numChars);

				numChars = 0;
				j++;
			}
			else /*Free allocated memory*/
			{
				free(buffer);
				buffer = NULL;
				for ( i = 0; i < j; i++)
				{
					free((*lines)[i]);
					(*lines)[i] = NULL;
				}
				free(*lines);
				*lines = NULL;
				return E_OUT_OF_MEMORY;
			}
			
		}

		numChars++;
	}
	
	free(buffer);
	buffer = NULL;

	return E_SUCCESS;
}

/**
 * Method:    createPA
 * @brief     Creates and initialises the PA struct for the process
 * @param 	  process - address of PA* to save to
 * @param 	  job - File name, should be in form PID-#
 * @return    ERROR_CODE enum type: E_SUCCESS if successful
 *									E_INVALID_INPUT if a file is in wrong format
 *									E_OUT_OF_MEMORY if a malloc was unsuccessful
 **/
ERROR_CODE createPA( Pa **process, char *job)
{
	int i, pid, numActivities;
	char state[4], format[12];/*1-3 for ac, 2 spaces, 3 for CPU or I/O and 3 for %d\n*/
	char **lines;
	
	/*Store array of activities in processes array*/
	if( getLines(job, &lines, &numActivities) != E_SUCCESS)
	{
		return E_INVALID_INPUT;
	}
	
	/*Create PA for first activity of each process, add to CPU queue*/
	(*process) = (Pa *) malloc( sizeof *(*process));
	if ( (*process) == NULL)
	{
		printf("Could not allocate memory for process\n");
		return E_OUT_OF_MEMORY;
	}
	
	if( sscanf(job, "PID-%d", &pid) != 1)
	{
		printf("Wrong format for job, %s, should be PID-#\n", job);
		return E_INVALID_INPUT;
	}
	(*process)->pid = pid;
	(*process)->arriveTime = 0;
	(*process)->totalAc = numActivities;
	(*process)->times = (int*) malloc( numActivities * sizeof(*((*process)->times))); 
	if ( (*process)->times == NULL)
	{
		printf("Couldn't allocate space for times\n");
		return E_OUT_OF_MEMORY;
	}
	
	/*Read in first line, to ensure file is in proper format*/
	if ( sscanf(lines[0], "%d CPU %d\n", &((*process)->ac), &((*process)->remainingTime)) != 2)
	{
		printf("Wrong format for activity line, %s, should be ac# CPU|I/O arrivetime#\n", lines[0]);
		free(lines[0]);
		lines[0] = NULL;
		return E_INVALID_INPUT;
	}
	free(lines[0]);
	lines[0] = NULL;

	(*process)->times[0] = (*process)->remainingTime;
	/*Read in remaining lines to get time for each activity*/
	for ( i = 1; i < numActivities; i++)
	{
		/*See if odd or even line*/
		if ( (i+1) % 2 == 0)
		{
			strncpy(state, "I/O", 4);
		}
		else 
		{
			strncpy(state, "CPU", 4);
		}
		/*Construct format for sscanf, where ac must be line number (i+1) and state
		* must be CPU if odd, I/O if even*/

		sprintf(format, "%d %3s %%d\n", i+1, state);
		if( sscanf(lines[i], format, &((*process)->times[i])) != 1)
		{
			printf("Wrong format for activity line, %s, should be %d %s arrivetime#\n", lines[i], i+1, state);
			free(lines[i]);
			lines[i] = NULL;
			return E_INVALID_INPUT;
		}

		free(lines[i]);
		lines[i] = NULL;
	}
	free(lines);
	lines = NULL;

	state[3] = '\0';
	if ( strcmp(state, "I/O") == 0)
	{
		(*process)->state = 1;
	}
	else if ( strcmp( state, "CPU") == 0)
	{
		(*process)->state = 2;
	}
	else
	{
		printf("Invalid identifier %3s", state);
	}

	return E_SUCCESS;
}