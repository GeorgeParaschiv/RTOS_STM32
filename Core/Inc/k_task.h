/*
 * k_task.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_TASK_H_
#define INC_K_TASK_H_
#include "common.h"

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 //PendSV is bits 23-16

void osKernelInit(void);
int osCreateTask(TCB* task) ;
int osKernelStart(void) ;
void osYield(void);
void osScheduler(void);
void osSleep(int timeInMs);
void osPeriodYield(void);
int osSetDeadline(int deadline, task_t TID);
int osCreateDeadlineTask(int deadline, int s_size, TCB* task);
int osTaskInfo(task_t TID, TCB* task_copy) ;
int osTaskExit(void);

#endif /* INC_K_TASK_H_ */
