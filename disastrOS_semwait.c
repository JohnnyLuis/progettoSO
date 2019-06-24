#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){
  int sem_fd = running->syscall_args[0];
  
  SemDescriptor* semDes = SemDescriptorList_byFd(&running->sem_descriptor,sem_fd);
  
  if (!semDes) {
	  running->syscall_retvalue = -1;
	  return;
  }
  
  Semaphore* sem = semDes->semaphore;
  
  (sem->count)--;
  
  if (sem->count < 0) {
	  running->status = Waiting;
	  List_insert(&waiting_list, waiting_list.last, (ListItem*)running);
	  List_insert(&(sem->waiting_descriptors), sem->waiting_descriptors.last, (ListItem*)semDes->ptr);
	  
	  running = List_detach(&ready_list,ready_list.first);
  }
}
