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
    disastrOS_debug("[SEMOPEN] id not valid\n");
	  running->syscall_retvalue = DSOS_ESEMOPEN;
	  return;
  }
  
  //count must be >0
  if (count <= 0) {
    disastrOS_debug("[SEMOPEN] count not valid\n");
	  running->syscall_retvalue = DSOS_ESEMOPEN;
	  return;
  }
  
  //when a process opens a new semaphore, it will be always created a new sem_descriptor: 
  //number of sem_descriptors per process must no exceed !
  if ((running->sem_descriptors.size)+1 > MAX_NUM_SEMDESCRIPTORS_PER_PROCESS) {
    disastrOS_debug("[SEMOPEN] MAX_NUM_SEMDESCRIPTORS_PER_PROCESS reached\n");
	  running->syscall_retvalue = DSOS_ESEMOPEN;
	  return;
  }
  
  //if a new semaphore is to be created, then the total number of exiting semaphores in the system must no exceed !
  if ((mode && DSOS_CREATE) && ((semaphores_list.size)+1 > MAX_NUM_SEMAPHORES)) {
    disastrOS_debug("[SEMOPEN] MAX_NUM_SEMAPHORES reached\n");
	  running->syscall_retvalue = DSOS_ESEMOPEN;
	  return;
  }

  //if the mode is DSOS_CREATE but it already exists a sem with id = id, returns an error.
  //viceversa, if the mode is not DSOS_CREATE but it doesn't exists a sem with id = id, returns en error.
  //if the checks go fine, I will have an opened semaphore (created if necessary) with id = id for the running process, in res
  Semaphore* sem=SemaphoreList_byId(&semaphores_list, id);
  if (mode && DSOS_CREATE) {
	if (sem) {
    disastrOS_debug("[SEMOPEN] semaphore with id=%d already exists. you can open it by using mode=0\n",id);
		running->syscall_retvalue = DSOS_ESEMOPEN;
		return;
	}
	sem = Semaphore_alloc(id,count);
  
	List_insert(&semaphores_list,semaphores_list.last,(ListItem*)sem);
  disastrOS_debug("[SEMOPEN] semaphore with id=%d successfully created\n",id);
  }
  else if (!sem) {
    disastrOS_debug("[SEMOPEN] semaphore with id=%d doesn't exists. you must create it before by using mode=DSOS_CREATE\n",id);
	  running->syscall_retvalue = DSOS_ESEMOPEN;
	  return;
  }
  
  //create a descriptor for the new opened semaphore
  SemDescriptor* semDes=SemDescriptor_alloc(running->last_sem_fd, sem, running);
  running->last_sem_fd++;
  SemDescriptorPtr* semDesPtr=SemDescriptorPtr_alloc(semDes);
  List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) semDes);

  semDes->ptr=semDesPtr;
  List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) semDesPtr);

  //return the sem_fd
  running->syscall_retvalue = semDes->fd;
  disastrOS_debug("[SEMOPEN] semaphore with id=%d successfully opened\n",id);
}
