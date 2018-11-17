/*
 ============================================================================
 Name        : dispatcher.c
 Author      : Kosta Kalenteridis
 Version     :
 Copyright   : Your copyright notice
 Description : Dispatcher for processes
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define CREATE 'C'
#define EXIT 'E'
#define REQUEST 'R'
#define INTERRUPT 'I'
#define TIMER_INTERRUPT 'T'

#define NEW 0
#define READY 1
#define RUNNING 2
#define BLOCKED 3
#define EXITED 4

typedef struct Process {
	int id;
	int startTime;
	int readyTime;
	int blockedTime;
	int runningTime;
	int status;
	struct Process *next;
} process;

typedef struct Queue {
	process *front;
	process *rear;
} queue;


process *newProcess(int, int);
queue *createQueue();
process *dequeue(queue*);
process *peek(queue*);
void enqueue(queue*, process*);
queue *display(queue*);
void startIdleProcess();
void processInput(char*, int);
void move_ready_process();
int is_empty(queue*);
void updateTime(int time);

//Making queues
queue blocked;
queue ready;
queue *new;

process *running;

process *idle_process;

process *processTable[512];
int processCount;
int idleTime;
int lastTime;

int main(void) {
	setvbuf(stdout, NULL, _IONBF, 0);

	processCount = 1;
	idleTime = 0;
	int systemTime;
	systemTime = 0;
	lastTime = 0;

	idle_process = (process*) malloc(sizeof(process));
	idle_process->id = 0;
	idle_process->startTime = 0;
	idle_process->runningTime = idleTime;
	startIdleProcess();
	processTable[0] = idle_process;

	char str[128];
	while (fgets(str, sizeof(str), stdin) != NULL) {
		if (strcmp(str, "\n\0") != 0) {
			processInput(str, systemTime);
		} else {
			break;
		}
	}
	int i;

	int tempId;
	int tempRunningTime;
	int tempReadyTime;
	int tempBlockedTime;

	printf("0 %d", idleTime);
	printf("\n");
	for (i = 1; i < processCount; i++) {
		tempId = processTable[i]->id;
		tempRunningTime = processTable[i]->runningTime;
		tempReadyTime = processTable[i]->readyTime;
		tempBlockedTime = processTable[i]->blockedTime;
		printf("%d %d %d %d", tempId, tempRunningTime, tempReadyTime,
				tempBlockedTime);
		printf("\n");
	}
	return EXIT_SUCCESS;
}

void updateTime(int time_passed) {
	process *tempProc;


	if (running->id == 0) {
		idleTime = idleTime + time_passed;
	}

	int i;
	for (i = 0; i < processCount; i++) {
		tempProc = processTable[i];
		if (tempProc->status == 3) {
			tempProc->blockedTime = tempProc->blockedTime + time_passed;
		}
		if (tempProc->status == 2) {
			tempProc->runningTime = tempProc->runningTime + time_passed;
		}
		if (tempProc->status == 1) {
			tempProc->readyTime = tempProc->readyTime + time_passed;
		}
	}
}


void startIdleProcess() {
	idle_process->runningTime = idleTime;
	idle_process->status = 2;
	running = idle_process;
}

void move_ready_process() {
	if (is_empty(&ready) == 1) {
		startIdleProcess();
	}
	else {
		process *next_running = dequeue(&ready);
		running = next_running;
		running->status = 2;
		//printf(next_running->id);
	}
}


queue *createQueue() {
	queue *q = (queue*) malloc(sizeof(queue));
	if (q == NULL)
		return NULL;
	q->front = NULL;
	q->rear = NULL;
	return q;
}

process *newProcess(int pid, int time) {
	process *proc;
	proc = (process*) malloc(sizeof(process));
	proc->id = pid;
	proc->startTime = time;
	proc->blockedTime = 0;
	proc->readyTime = 0;
	proc->runningTime = 0;
	if (running->id == 0) {
		running = proc;
		proc->status = 2;
	} else {
		enqueue(&ready, proc);
		proc->status = 1;
	}

	return proc;
}

void enqueue(queue *q, process *proc) {
	proc->next = NULL;
	if (q->front == NULL) {
		q->front = proc;
		q->rear = proc;
	} else {
		q->rear->next = proc;
		q->rear = proc;
	}
}

queue *display(queue *q) {
	process *ptr;
	ptr = q->front;
	if (ptr == NULL)
		printf("\n QUEUE IS EMPTY");
	else {
		printf("\n");
		while (ptr != q->rear) {
			printf("%d\t", ptr->id);
			ptr = ptr->next;
		}
		printf("%d\t", ptr->id);
	}
	return q;
}

process *dequeue(queue *q) {
	if (q == NULL || q->front == NULL)
		return NULL;
	process *proc;
	proc = q->front;
	q->front = q->front->next;
	return proc;
}

int is_empty(queue *q) {
	if (q->front == NULL) {
		return 1;
	} else {
		return 0;
	}
}

process *peek(queue *q) {
	if (q->front == NULL) {
		return -1;
	} else
		return q->front;
}

void processInput(char* str, int systemTime) {
	const char delims[] = " \n\t\r";

	int pid;

	int time = atoi(strtok(str, delims));
	char event = strtok(NULL, delims)[0];
	updateTime(time - lastTime);

	if (event == CREATE) {
		pid = atoi(strtok(NULL, delims));
		process *proc;
		proc = newProcess(pid, time);
		processTable[processCount] = proc;
		processCount = processCount + 1;
	}

	else if (event == EXIT) {
		pid = atoi(strtok(NULL, delims));
		running->status = 4;
		move_ready_process();
	}

	else if (event == REQUEST) {
		int resourceID = atoi(strtok(NULL, delims));
		pid = atoi(strtok(NULL, delims));
		int length = sizeof(processTable) / sizeof(process);
		process *tempProc;
		tempProc = running;
		tempProc->status = 3;
		move_ready_process();
	}

	else if (event == INTERRUPT) {
		int resourceID = atoi(strtok(NULL, delims));
		pid = atoi(strtok(NULL, delims));

		process *tempProc;
		int x;
		int tempID;
		for (x = 0; x < processCount; x++) {
			tempID = processTable[x]->id;
			if (tempID == pid) {
				tempProc = processTable[x];
			}
		}
		if (running->id == 0) {
			running = tempProc;
			running->status = 2;
		}
		else {
			tempProc->status = 1;
			enqueue(&ready, tempProc);

		}
	}

	else if (event == TIMER_INTERRUPT) {
		process *tempProc;
		if (running->id == 0) {
			printf("");
		}
		else {
			if (is_empty(&ready) == 0) {
				tempProc = running;
				tempProc->status = 1;
				enqueue(&ready, tempProc);
				move_ready_process();
			}
		}
	}

	lastTime = time;

}


