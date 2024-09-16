/*
 * k_task.c
 *
 *  Created on: Jan 31, 2024
 *      Author: naly5
 */
#include "k_task.h"
#include "main.h"
#include <limits.h>
#include "k_mem.h"
#include "common.h"
#include <stddef.h>

TCB tasks[MAX_TASKS];

uint32_t kernelInit = 0 ;
uint32_t kernelRunning = 0;

uint8_t currentTask = 0;
uint8_t newTask = 0;
uint8_t tempTask;

uint32_t * MSP_INIT_VAL;
uint32_t * current_stackptr;
uint32_t * scheduled_stackptr;
uint32_t stackUsed = 0;
uint32_t earliest_deadline;

void null_task(void){
	while(1){
	}
}

void osKernelInit(void)
{
	// Set Kernel Init Flag
	__disable_irq();
	kernelInit = 1 ;

	k_mem_init();

	// Initialize Main Stack Pointer
	MSP_INIT_VAL = * (uint32_t**) 0x0;

	SHPR3 |= 0xFFU << 24; //Set the priority of SysTick to be the weakest
	SHPR3 |= 0xFEU << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //set the priority of SVC higher than PendSV

	// Initialize TCB for all tasks
	for(int i = 0; i < MAX_TASKS; i++){
		tasks[i].tid = i;
		tasks[i].ptask = NULL;
		tasks[i].state = DORMANT;
		tasks[i].stack_high = 0;
		tasks[i].stack_size = STACK_SIZE;
		tasks[i].deadline_ms = 0;
		tasks[i].init_deadline = 5;
	}

	// Setup NULL task
	tasks[0].deadline_ms = INT_MAX;
	tasks[0].init_deadline = INT_MAX;
	tasks[0].ptask = &null_task;

	tasks[0].stack_p = k_mem_alloc(STACK_SIZE);
	tasks[0].stack_high = (uint32_t) tasks[0].stack_p + STACK_SIZE;
	tasks[0].stack_p = tasks[0].stack_high;

	*(--tasks[0].stack_p) = 1<<24;
	*(--tasks[0].stack_p) = (uint32_t) tasks[0].ptask;

	for (int j = 0; j < 14; ++j){
		*(--tasks[0].stack_p) = 0xA;
	}

	// Set task state to READY
	tasks[0].state = READY;
}

int osCreateTask(TCB* task) {
	return osCreateDeadlineTask(5, task->stack_size, task);
}

int osKernelStart(void) {

	SystemClock_Config();

	// Check if kernel not initialized or already running, if yes return RTX_ERR
	if (kernelInit == 0 || kernelRunning == 1) {
		return RTX_ERR ;
	}

	osScheduler();
	currentTask = newTask;

	__asm("SVC #1");

	return RTX_OK;
}

void osScheduler(void){

	__disable_irq();
	if (tasks[currentTask].state == RUNNING){
		tasks[currentTask].state = READY;
		tasks[currentTask].deadline_ms = tasks[currentTask].init_deadline;
	}
	earliest_deadline = tasks[0].deadline_ms;
	newTask = 0;
	scheduled_stackptr = tasks[0].stack_p;

	current_stackptr = tasks[currentTask].stack_p;

	for (int i = 0; i < MAX_TASKS; ++i){
		if (tasks[i].state == READY){
			if (tasks[i].deadline_ms < earliest_deadline){
				newTask = i;
				earliest_deadline = tasks[i].deadline_ms;
				scheduled_stackptr = tasks[i].stack_p;
			}
		}
	}

	tasks[newTask].state = RUNNING;
	kernelRunning = 1;

	__enable_irq();
}

void osYield(void){

	if (kernelInit == 0 || kernelRunning == 0) {
		return RTX_ERR ;
	}

	osScheduler();

	if (currentTask != newTask){
		__asm("SVC #2");
	}
}

void osSleep(int timeInMs){
	tasks[currentTask].state = SLEEPING;
	tasks[currentTask].deadline_ms = timeInMs;
	osYield();
}

void osPeriodYield(void){
	tasks[currentTask].state = SLEEPING;
	osYield();
}

int osSetDeadline(int deadline, task_t TID){
	__disable_irq();

	if (deadline <= 0){
		__enable_irq();
		return RTX_ERR;
	}

	if (tasks[TID].state != READY){
		__enable_irq();
		return RTX_ERR;
	}

	tasks[TID].deadline_ms = deadline;

	osYield();

	return RTX_OK;
}

int osCreateDeadlineTask(int deadline, int s_size, TCB* task){
	__disable_irq();
	// If Kernel uninitialized or tasks stack_size is too small return RTX_ERR
	if (kernelInit == 0) {
		return RTX_ERR;
	}

    int created = 0 ;

    // Loop to check if space for task to be created
    for(int i = 1; i < MAX_TASKS; i++){
        if (tasks[i].state == DORMANT) {

        	// Check if enough memory to allocate tasks stack to heap
        	tempTask = currentTask;
        	currentTask = i;
        	tasks[i].stack_p = k_mem_alloc(s_size);
        	currentTask = tempTask;

        	if (tasks[i].stack_p == NULL){ // No more room for new task
        		return RTX_ERR;
        	} else {

        		tasks[i].tid = i;
				task->tid = i;

				// Get task function pointer address
				tasks[i].ptask = task->ptask;

				// Get task stack size
				tasks[i].stack_size = s_size;
				tasks[i].init_deadline = deadline;
				tasks[i].deadline_ms = deadline;

				// Setup task stack
				tasks[i].stack_high = (uint32_t) tasks[i].stack_p + tasks[i].stack_size;
				tasks[i].stack_p = tasks[i].stack_high;

				*(--tasks[i].stack_p) = 1<<24;
				*(--tasks[i].stack_p) = (uint32_t) tasks[i].ptask;

				for (int j = 0; j < 14; ++j){
					*(--tasks[i].stack_p) = 0xA;
				}

				// Set task state to READY
				tasks[i].state = READY;

				// Update taskID and task created flag
				created = 1 ;

				if (kernelRunning == 1 && tasks[i].deadline_ms < tasks[currentTask].init_deadline) {
					osYield();
				}

				break ;

        	}
        }
    }

    __enable_irq();

    // If task maximum reached return RTX_ERR
    if (created == 0) {
        return RTX_ERR ;
    }

    // If tasks created successfully return RTX_OK
    return RTX_OK ;

}


int osTaskInfo(task_t TID, TCB* task_copy) {

	if (kernelInit == 0 || tasks[TID].state == DORMANT) {
		return RTX_ERR ;
	}

	if (TID < MAX_TASKS) {
		task_copy->ptask = tasks[TID].ptask ;
		task_copy->stack_high = tasks[TID].stack_high;
		task_copy->stack_size = tasks[TID].stack_size;
		task_copy->stack_p = tasks[TID].stack_p;
		task_copy->state = tasks[TID].state ;
		task_copy->tid = tasks[TID].tid;
		task_copy->init_deadline = tasks[TID].init_deadline;
		task_copy->deadline_ms = tasks [TID].deadline_ms ;
		return RTX_OK ;
	}


	return RTX_ERR;
}

int osTaskExit(void){
	if (kernelInit == 0 || kernelRunning == 0) {
		return RTX_ERR ;
	}
	// Check if was called by a running task, if not return RTC_ERR
	if (tasks[currentTask].state != RUNNING)
		return RTX_ERR;

	// Set state to DORMANT and save return RTX_OK
	k_mem_dealloc(tasks[currentTask].stack_high - tasks[currentTask].stack_size);
	tasks[currentTask].stack_high = 0;
	tasks[currentTask].state = DORMANT;

	osYield();

	return RTX_OK;
}
