#ifndef PA_H
#define PA_H
typedef struct Pa{
 /**Process ID, same as file PID-#*/
 int pid;
 /**Next activity line to execute*/
 int ac;
 /**Total number of activities*/
 int totalAc;
 /**Current need of process, 1 = I/O, 2 = CPU*/
 int state;  
 /**Time the process arrive to CPU queue or I/O queue*/
 int arriveTime; 
 /**Burst time for activity of process*/
 int remainingTime; 
 /**Array of times for each activity*/
 int *times;
} Pa;
#endif