/*
 * k_mem.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_MEM_H_
#define INC_K_MEM_H_
#include "common.h"
#include <stddef.h>

int k_mem_init();
void* k_mem_alloc(size_t size);
int k_mem_dealloc(void* ptr);
int k_mem_count_extfrag(size_t size);

#define FREE 0
#define ALLOCATED 1

typedef struct header{
    uint8_t type;
    uint32_t size;
    task_t owner;
}header;

typedef struct free_header{
    struct header metadata;
    struct free_header *next, *prev;
}free_header;

#endif /* INC_K_MEM_H_ */
