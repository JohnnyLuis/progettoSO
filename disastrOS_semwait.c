#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){
  int sem_fd = running->syscall_args[0];
  
  SemDescriptor* semDes = SemDescriptorList_byFd(&running->sem_descriptors,sem_fd);
  
  if (!semDes) {
	  running->syscall_retvalue = -1;
	  return;
  }
  
  Semaphore* sem = semDes->semaphore;
  
  (sem->count)--;
  
  if (sem->count < 0) {
    disastrOS_debug("[SEMWAIT] current running process sets to waiting");
	  running->status = Waiting;
	  List_insert(&waiting_list, waiting_list.last, (ListItem*)running);
	  List_insert(&(sem->waiting_descriptors), sem->waiting_descriptors.last, (ListItem*)semDes->ptr);
	  
	  running = (PCB*) List_detach(&ready_list,ready_list.first);
  }
  
  running->syscall_retvalue = 0;
  disastrOS_debug("[SEMWAIT] semwait successfully completed. current count for sem_id=%d is %d",semDes->semaphore->id,semDes->semaphore->count);
}
