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
  ucontext_t *tmp;

  tmp = running;
  running = ready;
  ready = tmp;

  swapcontext(ready, running);
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
