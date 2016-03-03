#ifndef TASK_H
#define TASK_H

typedef struct _task {
	signed char startState;
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;

#endif