#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
  int sem_fd = running->syscall_args[0];
  
  SemDescriptor* semDes=SemDescriptorList_byFd(&running->sem_descriptors, sem_fd);
  
  //if the descriptor sem_fd is not found in sem_descriptors list of the running process, returns an error
  if (!semDes) {
    disastrOS_debug("[SEMCLOSE] descriptor not found in this process\n");
    running->syscall_retvalue=DSOS_ESEMCLOSE;
    return;
  }
  
  //if a process wants to close a semaphore, it's necessary to remove the descriptor, associated to the semaphore, from 
  //the process' descriptor list ...
  List_detach(&running->sem_descriptors, (ListItem*) semDes);
  
  Semaphore* sem=semDes->semaphore;
  
  //... and then to remove the descriptor pointer and the descriptor, referred to the process, from the semaphore's descriptors lists.
  //finally, SemDescriptor and SemDescriptorPtr structers are deallocated
  SemDescriptorPtr* semDesPtr=(SemDescriptorPtr*) List_detach(&sem->descriptors, (ListItem*)(semDes->ptr));
  SemDescriptor_free(semDes);
  SemDescriptorPtr_free(semDesPtr);
  disastrOS_debug("[SEMCLOSE] pid=%d closed sem_id=%d\n",running->pid,sem->id);
  
  //if the semaphore that the process has just closed, it is no longer opened by any process, will be destroyed:
  //so it will be removed from semaphores list and deallocated its structure
  if (! sem->descriptors.size) {
    List_detach(&semaphores_list, (ListItem*)sem);
    Semaphore_free(sem);
    disastrOS_debug("[SEMCLOSE] sem_id=%d destroyed\n",sem->id);
  }
    
  running->syscall_retvalue=0;
}
