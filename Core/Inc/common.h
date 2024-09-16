/*
 * common.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: If you feel that there are common
 *      C functions corresponding to this
 *      header, then any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#define MAIN_STACK_SIZE 0x1000

#define TID_NULL 0 //predefined Task ID for the NULL task
#define MAX_TASKS 16 //maximum number of tasks in the system
#define STACK_SIZE 0x400 //min. size of each task’s stack
#define DORMANT 0 //state of terminated task
#define READY 1 //state of task that can be scheduled but is not running
#define RUNNING 2 //state of running task
#define SLEEPING 3 // state of sleeping task

#define RTX_ERR 0
#define RTX_OK 1

#define _ICSR *(uint32_t*)0xE000ED04

#include <stdio.h>
#include <stdint.h>
#define task_t uint32_t

typedef struct task_control_block{
    void (*ptask)(void* args); //entry address
    uint32_t stack_high; //start starting address (high address)
    task_t tid; //task ID
    uint8_t state; //task's state
    uint16_t stack_size; //stack size. Must be a multiple of 8
    uint32_t * stack_p;
    uint32_t init_deadline;
    uint32_t deadline_ms;
}TCB;

#endif /* INC_COMMON_H_ */
