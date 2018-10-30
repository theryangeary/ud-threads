#include "t_lib.h"

struct tcb {
  int         thread_id;
  int         thread_priority;
  ucontext_t  thread_context;
  struct tcb *next;
};

typedef struct tcb tcb;

ucontext_t *running;
ucontext_t *ready;
tcb* readyQueue;
tcb* runningQueue;

void t_yield()
{
  tcb* current = readyQueue;
  if (NULL == current) {
    runningQueue = readyQueue;
    return;
  }
  else {
    while(NULL != current->next) {
      current = current->next;
    }
    current->next = runningQueue;
    runningQueue = readyQueue;
    readyQueue = readyQueue->next;
    runningQueue->next = NULL;

    /* ucontext_t *tmp; */

    /* tmp = running; */
    /* running = ready; */
    /* ready = tmp; */

    swapcontext(&readyQueue->thread_context, &runningQueue->thread_context);
  }
}

void t_init()
{
  readyQueue = (tcb*) malloc(sizeof(tcb));
  runningQueue = (tcb*) malloc(sizeof(tcb));
  ucontext_t *tmp;
  tmp = (ucontext_t *) malloc(sizeof(ucontext_t));

  getcontext(tmp);    /* let tmp be the context of main() */
  running = tmp;
  runningQueue->thread_id = 0;
  runningQueue->thread_priority = 0;
  runningQueue->thread_context = *tmp;
  runningQueue->next = NULL;
  readyQueue = NULL;
}

int t_create(void (*fct)(int), int id, int pri)
{
  size_t sz = 0x10000;

  ucontext_t *uc;
  uc = (ucontext_t *) malloc(sizeof(ucontext_t));

  getcontext(uc);
/***
  uc->uc_stack.ss_sp = mmap(0, sz,
       PROT_READ | PROT_WRITE | PROT_EXEC,
       MAP_PRIVATE | MAP_ANON, -1, 0);
***/
  uc->uc_stack.ss_sp = malloc(sz);  /* new statement */
  uc->uc_stack.ss_size = sz;
  uc->uc_stack.ss_flags = 0;
  uc->uc_link = running;
  makecontext(uc, (void (*)(void)) fct, 1, id);
  ready = uc;

  tcb* tmp = (tcb*) malloc(sizeof(tcb));
  tmp->thread_id = id;
  tmp->thread_priority = pri;
  tmp->thread_context = *uc;

  tcb* current = readyQueue;
  if(NULL == current) {
    current = tmp;
  }
  else {
    while(NULL != current->next) {
      current = current->next;
    }
    current->next = tmp;
  }
  return 0;
}

void t_shutdown() {
  tcb* current = runningQueue;
  tcb* next = current;
  while(NULL != current) {
    next = current->next;
    free(current);
    current = next;
  }
}

void t_terminate() {
  tcb* toRemove = runningQueue;
  runningQueue = runningQueue->next;
  free(toRemove);
  runningQueue = readyQueue;
  readyQueue = readyQueue->next;
  runningQueue->next = NULL;
  setcontext(&runningQueue->thread_context);
}

