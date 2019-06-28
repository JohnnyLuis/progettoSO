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
  
  //if the descriptor sem_fd is not found in sem_descriptors list of the running process, returns an error
  if (!semDes) {
    disastrOS_debug("[SEMWAIT] descriptor not found in this process\n");
	  running->syscall_retvalue = DSOS_ESEMWAIT;
	  return;
  }
  
  Semaphore* sem = semDes->semaphore;
  
  (sem->count)--;
  
  //after the decrease of sem count, if this value is <0 then the running process must be blocked and put into waiting queue of the semaphore (waiting_descriptors)
  if (sem->count < 0) {
    disastrOS_debug("[SEMWAIT] current running process sets to waiting\n");
    
    //current running process is blocked and so it is put between the processes in the waiting list 
	  running->status = Waiting;
    List_insert(&waiting_list, waiting_list.last, (ListItem*)running); 
    
    //the descriptor pointer associated to the descriptor, that identifies the process, is removed from the descriptors list of the semaphore and then it is inserted
    //into the waiting descriptors list
    SemDescriptorPtr* semDesPtr = (SemDescriptorPtr*)List_detach(&(sem->descriptors),(ListItem*)semDes->ptr); 
    List_insert(&(sem->waiting_descriptors), sem->waiting_descriptors.last, (ListItem*)semDesPtr);
    
    //since the current running has been blocked, the new running process is the next one in ready queue (ready_list)
    running = (PCB*) List_detach(&ready_list,ready_list.first);
  }
  
  running->syscall_retvalue = 0;
  disastrOS_debug("[SEMWAIT] semwait successfully completed. current count for sem_id=%d is %d\n",semDes->semaphore->id,semDes->semaphore->count);
}
