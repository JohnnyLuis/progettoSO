#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
  int sem_fd = running->syscall_args[0];
  
  SemDescriptor* semDes=SemDescriptorList_byFd(&running->sem_descriptors, fd);
  
  if (! semDes){
    running->syscall_retvalue=-1;
    return;
  }
  
  List_detach(&running->sem_descriptors, (ListItem*) semDes);
  
  Semaphore* sem=semDes->semaphore;
  
  SemDescriptorPtr* semDesPtr=(SemDescriptorPtr*) List_detach(&sem->descriptors, (ListItem*)(sem->ptr));
  SemDescriptor_free(semDes);
  SemDescriptorPtr_free(semDesPtr);
  running->syscall_retvalue=0;
}
