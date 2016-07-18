#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "linkedList.h"
#include "pa.h"
ERROR_CODE scheduler(char** job, int numJobs);
void* cpu(void*);
void* io(void*);
ERROR_CODE getLines(char *job, char ***array, int *numJobs);
ERROR_CODE createPA( Pa **process, char *job);
#endif