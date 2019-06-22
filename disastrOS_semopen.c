#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen(){
  int id = running->syscall_args[0];
  int count = 1;

  Semaphore* res=SemaphoreList_byId(&semaphores_list, id);
  if (!res) {
	res = Semaphore_alloc(id,count);
	List_insert(&semaphores_list,semaphores_list.last,(ListItem*)res);
  }

  SemDescriptor* des=SemDescriptor_alloc(running->last_fd, res, running);
  running->last_fd++;
  SemDescriptorPtr* desptr=SemDescriptorPtr_alloc(des);
  List_insert(&running->descriptors, running->descriptors.last, (ListItem*) des);

  des->ptr=desptr;
  List_insert(&res->descriptors, res->descriptors.last, (ListItem*) desptr);

  running->syscall_retvalue = des->fd;
}
