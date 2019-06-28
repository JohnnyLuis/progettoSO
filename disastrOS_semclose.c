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
  
  if (! semDes){
    disastrOS_debug("[SEMCLOSE] descriptor not found in this process\n");
    running->syscall_retvalue=-1;
    return;
  }
  
  List_detach(&running->sem_descriptors, (ListItem*) semDes);
  
  Semaphore* sem=semDes->semaphore;
  
  SemDescriptorPtr* semDesPtr=(SemDescriptorPtr*) List_detach(&sem->descriptors, (ListItem*)(semDes->ptr));
  SemDescriptor_free(semDes);
  SemDescriptorPtr_free(semDesPtr);
  disastrOS_debug("[SEMCLOSE] pid=%d closed sem_id=%d\n",running->pid,sem->id);
  
  if (! sem->descriptors.size) {
    List_detach(&semaphores_list, (ListItem*)sem);
    Semaphore_free(sem);
    disastrOS_debug("[SEMCLOSE] sem_id=%d destroyed\n",running->pid,sem->id);
  }
    
  running->syscall_retvalue=0;
}
