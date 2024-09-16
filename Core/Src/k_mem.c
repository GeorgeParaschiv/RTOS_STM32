#include "k_mem.h"
#include "k_task.h"

extern uint32_t _img_end;
extern uint32_t _estack;
extern uint32_t _Min_Stack_Size;

uint32_t *start_of_heap;
uint32_t *end_of_heap;

free_header *head;
uint32_t memInit = 0;
extern uint32_t kernelInit;
extern uint8_t currentTask;


int k_mem_init(){
    if(kernelInit == 0){
        return RTX_ERR;
    }

    if(memInit == 1){
        return RTX_ERR;
    }

    // Get addresses for start and end of heap
    start_of_heap = &_img_end;
    end_of_heap = (uint32_t) &_estack - (uint32_t) &_Min_Stack_Size;

    // Creating head of the heap
    head = start_of_heap;
    head->metadata.owner = 0;
    head->metadata.type = FREE;
    head->metadata.size = (uint32_t) end_of_heap - (uint32_t) start_of_heap;
    head->next = NULL;
    head->prev = NULL;

    memInit = 1;
    return RTX_OK;
}



void* k_mem_alloc(size_t size){

	// If memory not initialized or size is 0 return NULL
	if (memInit == 0 || size == 0){
		return NULL;
	}

	// First round up size to nearest multiple of 4 for byte alignment
	uint32_t allocate_size = ((size + 3) & ~0x03) + sizeof(free_header);

	// Next iterate through linked list until we find a section large enough
	free_header * temp = head;

	while (temp != NULL){

		if (temp->metadata.size >= allocate_size){

			free_header * allocated = temp;

			// Determine if size left over large enough to be useful
			if (temp->metadata.size - allocate_size >= sizeof(free_header) + 4){

				// If yes shift free up and update linked list
				temp = (uint32_t) temp + allocate_size;


				temp->metadata = allocated->metadata;
				temp->metadata.size = temp->metadata.size - allocate_size;
				temp->next = allocated->next;
				temp->prev = allocated->prev;

				if (allocated->prev == NULL){ // Temp is first pointer in list
					if (allocated->next != NULL){ // If first pointer in list update the next pointer and the nexts previous pointer
						head = temp;
						head->next->prev = temp;
					} else { //only 1 free block
						head = temp;
					}
				} else {
					if (allocated->next == NULL) {// If last pointer in list update the prev pointer and the previouses next pointer
						temp->prev = allocated->prev;
						temp->prev->next = temp;
					} else { // If both next and previous pointer exist update everything
						temp->next = allocated->next;
						temp->prev = allocated->prev;
						temp->next->prev = temp;
						temp->prev->next = temp;
					}
				}

				allocated->metadata.owner = currentTask;
				allocated->metadata.type = ALLOCATED;
				allocated->metadata.size = allocate_size;
				allocated->next = NULL;
				allocated->prev = NULL;
			} else {

				// If not enough left over space just allocate the whole thing
				allocate_size = temp->metadata.size;


				if (temp->prev == NULL){ // Temp is first free block
					if (temp->next == NULL){ // Temp is the only free block
						head = NULL;
					} else { // Temp is first free block
						head = temp->next;
						head->prev = NULL;
					}
				} else {
					if (temp->next == NULL){ // Temp is the last free block
						temp->prev->next = NULL;
						temp = NULL;
					} else { // Temp is some middle block;
						temp->prev->next = temp->next;
						temp->next->prev = temp->prev;
						temp = NULL;
					}
				}

				allocated->metadata.owner = currentTask;
				allocated->metadata.type = ALLOCATED;
				allocated->metadata.size = allocate_size;
				allocated->next = NULL;
				allocated->prev = NULL;
			}
			return (uint32_t) allocated + sizeof(free_header);
		} else {
			temp = temp->next;
		}
	}
	return NULL;
}




int k_mem_dealloc(void* ptr){

	// If ptr NULL return RTX_OK
	if (ptr == NULL) return RTX_OK;

	if ((uint32_t) ptr - sizeof(free_header) < (uint32_t) start_of_heap || (uint32_t) ptr - sizeof(free_header) > (uint32_t) end_of_heap) return RTX_ERR;

	// Find starting address of metadata structure
	free_header * address = (uint32_t) ptr - sizeof(free_header);

	// Ensure that this is a valid starting point and that we are the owner and the memory is allocated
	if (address->metadata.owner == currentTask && address->metadata.type == ALLOCATED){
		address->metadata.type = FREE;
		address->metadata.owner = 0;

		// Find the first free block before this one
		free_header * temp = head;

		// If this come before first free block it is the new head
		if ((uint32_t) address < (uint32_t) head){
			address->next = head;
			address->prev = NULL;
			head = address;

			// Now check if the old head block can be coalesced
			if ((uint32_t) head + head->metadata.size == (uint32_t) temp){
				head->next = temp->next;
				if (head->next != NULL) head->next->prev = head;
				head->metadata.size += temp->metadata.size;
			} else {
				head->next->prev = head;
			}
		} else {
			while (temp != NULL){

				// If temp is greater than the address, we have found the very next free block
				if ((uint32_t) temp < (uint32_t) address){
					address->prev = temp;
					address->next = temp->next;
				} else {
					break;
				}

				temp=temp->next;
			}

			// Check if can coalesce with next block first
			if (address->next != NULL){
				if ((uint32_t) address + address->metadata.size == (uint32_t) address->next){
					address->metadata.size += address->next->metadata.size;
					address->next = address->next->next;

					if (address->next != NULL){
						address->next->prev = address;
					}

				} else {
					address->next->prev = address;
				}
			}

			// Check if can coalesce with previous block
			if ((uint32_t) address->prev + address->prev->metadata.size == (uint32_t) address){
				address->prev->metadata.size += address->metadata.size;
				address->prev->next = address->next;

				if (address->next != NULL){
					address->next->prev = address->prev;
				}

			} else {
				address->prev->next = address;
			}

		}

		return RTX_OK;
	}
	return RTX_ERR;
}




int k_mem_count_extfrag(size_t size){
    // If memory has not been initialized
    if (memInit == 0){
        return 0;
    }

    free_header * temp = head;
    int counter = 0;
    while(temp != NULL){
        if(temp->metadata.size < size){
            counter += 1;
        }
        temp = temp -> next;
    }

    return counter;
}
