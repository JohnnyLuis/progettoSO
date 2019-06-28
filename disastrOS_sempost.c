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
  
	if (!semDes) {
		disastrOS_debug("[SEMPOST] descriptor not found in this process\n");
		running->syscall_retvalue = -1;
		return;
	}
  
	Semaphore* sem = semDes->semaphore;
	
	(sem->count)++;
	
	if (sem->count <= 0) {
		SemDescriptorPtr* semDesPtr = (SemDescriptorPtr*) List_detach(&(sem->waiting_descriptors), (ListItem*)sem->waiting_descriptors.first);
		List_insert(&(sem->descriptors), sem->descriptors.last, (ListItem*)semDesPtr);
		PCB* ready_process = semDesPtr->descriptor->pcb;
		List_detach(&waiting_list, (ListItem*)ready_process);
		ready_process->status = Ready;
		List_insert(&ready_list, ready_list.last, (ListItem*)ready_process);
	}
	
	running->syscall_retvalue = -1;
	disastrOS_debug("[SEMPOST] sempost successfully completed. current count for sem_id=%d is %d\n",semDes->semaphore->id,semDes->semaphore->count);
}
