#include <stdio.h>
#include <unistd.h>
#include <poll.h>



#include "disastrOS.h"



// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void testMaxNumSemdescriptorsPerProcessFunction(void* args) {
  int sem_count = 1;
  int mode = DSOS_CREATE;
  int sem_fd_array[MAX_NUM_SEMDESCRIPTORS_PER_PROCESS + 1];
  
  printf("last semopen should return an error\n");
  for (int i=0 ; i <= MAX_NUM_SEMDESCRIPTORS_PER_PROCESS ; i++) {
    sem_fd_array[i] = disastrOS_semOpen(i+15,sem_count,mode);
  }
  
  disastrOS_printStatus();
  
  for (int i=0 ; i <= MAX_NUM_SEMDESCRIPTORS_PER_PROCESS ; i++) {
    disastrOS_semClose(sem_fd_array[i]);
  }
  
  disastrOS_printStatus();
  
  disastrOS_exit(disastrOS_getpid()+1);
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int sem_fd;
  int type=0;
  int mode=0;
  int sem_count = 1;
  int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
  printf("fd=%d\n", fd);
  
  //child processes with odd pid will try to create semaphores that already exist ...
  if (disastrOS_getpid() % 2) {
    mode = DSOS_CREATE;
    sem_fd = disastrOS_semOpen(disastrOS_getpid(),sem_count,mode);
    
    //... moreover, child #5 and child #7 will open semaphore #6 and then they will execute a semwait and will be blocked between
    //semwait and sempost instructions: in this way, the next process who executes semwait on sem #6 will be put into the waiting queue
    //of the semaphore...
    if (disastrOS_getpid() == 5 || disastrOS_getpid() == 7) {
      mode = 0;
      
      sem_fd = disastrOS_semOpen(6,sem_count,mode);
      disastrOS_semWait(sem_fd);
      disastrOS_sleep(4);
      disastrOS_semPost(sem_fd);
    
    }
  }
  
  //child processes with even pid will open semaphores that's been already created. then they will execute a semwait and a sempost
  else {
    sem_fd = disastrOS_semOpen(disastrOS_getpid(),sem_count,mode);
    disastrOS_semWait(sem_fd);
    disastrOS_semPost(sem_fd);
  }
  
  disastrOS_semClose(sem_fd);
  
  printf("PID: %d, terminating\n", disastrOS_getpid());
  /*
  for (int i=0; i<(disastrOS_getpid()+1); ++i){
    printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
    disastrOS_sleep((20-disastrOS_getpid())*5);
  }*/
  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);
  int type = 0;
  int mode=DSOS_CREATE;
  int sem_count = 1;

  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  for (int i=0; i<10; ++i) {
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    int fd=disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd);
	
    //creating new semaphores...
	  printf("opening semaphore (and creating if necessary)\n");
	  int sem_fd=disastrOS_semOpen(i,sem_count,mode);
    printf("sem_fd=%d\n\n", sem_fd);
	
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }
  
  //I try to open a semaphore that it's never been created before
  mode = 0;
  printf("opening semaphore\n");
	int sem_fd=disastrOS_semOpen(50,sem_count,mode);
  printf("sem_fd=%d\n\n", sem_fd);
  
  disastrOS_spawn(testMaxNumSemdescriptorsPerProcessFunction, 0);
  alive_children++;

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }
  disastrOS_printStatus();
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
