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

void t_yield()
{
  tcb* current = readyQueue;

  current = readyQueue;
  if (NULL == readyQueue) {
    runningQueue = readyQueue;
    return;
  }
  else {
    while(NULL != current->next) {
      current = current->next;
    }
    current->next = runningQueue;
    tcb* last = runningQueue;
    runningQueue = readyQueue;
    readyQueue = readyQueue->next;
    runningQueue->next = NULL;
    current = readyQueue;

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
  runningQueue->thread_priority = 0;
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

void t_shutdown() {
  tcb* current = readyQueue;
  tcb* next = current;
  while(NULL != current) {
    next = current->next;
    free(current->thread_context->uc_stack.ss_sp);
    free(current->thread_context);
    free(current);
    current = next;
  }
  if (NULL != runningQueue) {
    free(runningQueue->thread_context->uc_stack.ss_sp);
    free(runningQueue->thread_context);
    free(runningQueue);
  }
}

void t_terminate() {
  if (runningQueue != NULL) {
    free(runningQueue->thread_context->uc_stack.ss_sp);
    free(runningQueue->thread_context);
    free(runningQueue);
  }
  runningQueue = readyQueue;
  readyQueue = readyQueue->next;
  runningQueue->next = NULL;
  setcontext(runningQueue->thread_context);
}

