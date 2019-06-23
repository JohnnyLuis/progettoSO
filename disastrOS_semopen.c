#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen(){
  int id = running->syscall_args[0];
  int count = running->syscall_args[1];
  int mode = running->syscall_args[2];
  
  //id must be >=0
  if (id < 0) {
	running->syscall_retvalue = -1;
	return;
  }
  
  //when a process opens a new semaphore, it will be always created a new sem_descriptor: 
  //number of sem_descriptors per process must no exceed !
  if ((running->sem_descriptors.size)+1 > MAX_NUM_SEMDESCRIPTORS_PER_PROCESS) {
	  running->syscall_retvalue = -1;
	  return;
  }
  
  //if a new semaphore is to be created, then the total number of exiting semaphores in the system must no exceed !
  if ((mode && DSOS_CREATE) && ((semaphores_list.size)+1 > MAX_NUM_SEMAPHORES)) {
	  running->syscall_retvalue = -1;
	  return;
  }

  //if the mode is DSOS_CREATE but it already exists a sem with id = id, returns an error.
  //viceversa, if the mode is not DSOS_CREATE but it doesn't exists a sem with id = id, returns en error.
  //if the checks go fine, I will have an opened semaphore (created if necessary) with id = id for the running process, in res
  Semaphore* res=SemaphoreList_byId(&semaphores_list, id);
  if (mode && DSOS_CREATE) {
	if (res) {
		running->syscall_retvalue = -1;
		return;
	}
	res = Semaphore_alloc(id,count);
	List_insert(&semaphores_list,semaphores_list.last,(ListItem*)res);
  }
  else if (!res) {
	  running->syscall_retvalue = -1;
	  return;
  }

  //create a descriptor for the new opened semaphore
  SemDescriptor* des=SemDescriptor_alloc(running->last_sem_fd, res, running);
  running->last_sem_fd++;
  SemDescriptorPtr* desptr=SemDescriptorPtr_alloc(des);
  List_insert(&running->descriptors, running->descriptors.last, (ListItem*) des);

  des->ptr=desptr;
  List_insert(&res->descriptors, res->descriptors.last, (ListItem*) desptr);

  //return the sem_fd
  running->syscall_retvalue = des->fd;
}
