#include "t_lib.h"

struct tcb {
  int         thread_id;
  int         thread_priority;
  ucontext_t* thread_context;
  struct tcb *next;
};

typedef struct tcb tcb;

tcb* readyQueue;
tcb* runningQueue;

tcb* insert(tcb* head, tcb* newTcb) {
  // check first case
  if(newTcb->thread_priority < head->thread_priority) {
    newTcb->next = head;
    return newTcb;
  }
  // check rest
  tcb* current = head;
  // find last queue item with this priority
  while(current->next != NULL && current->next->thread_priority <= newTcb->thread_priority){
    current = current->next;
  }
  // insert item
  newTcb->next = current->next;
  current->next = newTcb;
  return head;
}

void t_yield()
{
  tcb* current = readyQueue;

  current = readyQueue;
  if (NULL == readyQueue) {
    runningQueue = readyQueue;
    return;
  }
  else {
    tcb* last = runningQueue;
    readyQueue = insert(readyQueue, runningQueue);
    runningQueue = readyQueue;
    readyQueue = readyQueue->next;
    runningQueue->next = NULL;

    swapcontext(last->thread_context, runningQueue->thread_context);
  }
}

void t_init()
{
  runningQueue = (tcb*) malloc(sizeof(tcb));
  ucontext_t *tmp;
  tmp = (ucontext_t *) malloc(sizeof(ucontext_t));

  getcontext(tmp);    /* let tmp be the context of main() */
  runningQueue->thread_id = 0;
  runningQueue->thread_priority = 1;
  runningQueue->thread_context = tmp;
  runningQueue->next = NULL;
  readyQueue = NULL;
  /* free(tmp); */
}

int t_create(void (*fct)(int), int id, int pri)
{
  size_t sz = 0x10000;

  ucontext_t *uc;
  uc = (ucontext_t *) malloc(sizeof(ucontext_t));

  getcontext(uc);
  uc->uc_stack.ss_sp = malloc(sz);  /* new statement */
  uc->uc_stack.ss_size = sz;
  uc->uc_stack.ss_flags = 0;
  makecontext(uc, (void (*)(void)) fct, 1, id);

  tcb* tmp = (tcb*) malloc(sizeof(tcb));
  tmp->thread_id = id;
  tmp->thread_priority = pri;
  tmp->thread_context = uc;

  tcb* current = readyQueue;
  if(NULL == readyQueue) {
    readyQueue = tmp;
  }
  else {
    while(NULL != current->next) {
      current = current->next;
    }
    current->next = tmp;
  }
  /* free(uc); */
  return 0;
}

void freeTcb(tcb* tmp) {
  free(tmp->thread_context->uc_stack.ss_sp);
  free(tmp->thread_context);
  free(tmp);
}

void t_shutdown() {
  tcb* current = readyQueue;
  tcb* next = current;
  while(NULL != current) {
    next = current->next;
    freeTcb(current);
    current = next;
  }
  if (NULL != runningQueue) {
    freeTcb(runningQueue);
  }
}

void t_terminate() {
  if (runningQueue != NULL) {
    freeTcb(runningQueue);
  }
  runningQueue = readyQueue;
  readyQueue = readyQueue->next;
  runningQueue->next = NULL;
  setcontext(runningQueue->thread_context);
}

