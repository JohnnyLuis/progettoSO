#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
	 int sem_fd = running->syscall_args[0];
	 
	 SemDescriptor* semDes = SemDescriptorList_byFd(&running->sem_descriptors,sem_fd);
  
	//if the descriptor sem_fd is not found in sem_descriptors list of the running process, returns an error
	if (!semDes) {
		disastrOS_debug("[SEMPOST] descriptor not found in this process\n");
		running->syscall_retvalue = DSOS_ESEMPOST;
		return;
	}
  
	Semaphore* sem = semDes->semaphore;
	
	(sem->count)++;
	
	//after the increase of sem count, if this value is <=0 it means there's at least one process in the queue of the semaphore.
	//so it's necessary to remove the next process in the semaphore's queue from this one and insert it in the ready process list
	if (sem->count <= 0) {
		
		//the descriptor pointer associated to the descriptor, that identifies the next process in the semaphore's queue, is removed from 
		//the waiting descriptors list of the semaphore and then it is inserted into the descriptors list of the semaphore
		SemDescriptorPtr* semDesPtr = (SemDescriptorPtr*) List_detach(&(sem->waiting_descriptors), (ListItem*)sem->waiting_descriptors.first);
		List_insert(&(sem->descriptors), sem->descriptors.last, (ListItem*)semDesPtr);
		
		//remove the new ready process from the waiting list and insert it into ready list
		PCB* ready_process = semDesPtr->descriptor->pcb;
		List_detach(&waiting_list, (ListItem*)ready_process);
		ready_process->status = Ready;
		List_insert(&ready_list, ready_list.last, (ListItem*)ready_process);
	}
	
	running->syscall_retvalue = 0;
	disastrOS_debug("[SEMPOST] sempost successfully completed. current count for sem_id=%d is %d\n",semDes->semaphore->id,semDes->semaphore->count);
}
