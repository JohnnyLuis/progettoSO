#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
	 int sem_fd = running->syscall_args[0];
	 
	 SemDescriptor* semDes = SemDescriptorList_byFd(&running->sem_descriptor,sem_fd);
  
	if (!semDes) {
		running->syscall_retvalue = -1;
		return;
	}
  
	Semaphore* sem = semDes->semaphore;
	
	(sem->count)++;
	
	if (sem->count > 0 && sem->waiting_descriptors.size) {
		SemDescriptorPtr* semDesPtr = List_detach(&sem->waiting_descriptors, sem->waiting_descriptors.first);
		PCB* ready_process = semDesPtr->descriptor.pcb;
		List_detach(&waiting_list, ready_process);
		ready_process->status = Ready;
		List_insert(&ready_list, ready_list.last, (ListItem*)ready_process);
	}
	
	running->syscall_retvalue = -1;
}
