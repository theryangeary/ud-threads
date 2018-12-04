// Writen by Ryan Geary and James Skripchuk (c) 2018

#include "t_lib.h"
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

const useconds_t interval = 500;
tcb* readyQueue;
tcb* runningQueue;
mbox* tid_map[100];


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
  // entering critical section
  sighold(SIGALRM);
  // cancel any upcoming alarm, if thread exited before its time was up
  //ualarm(0,0);
  // set new alarm for next thread
  //useconds_t remainingTime = ualarm(interval,0);

  tcb* current = readyQueue;

  current = readyQueue;
  // if ready is empty, move running to ready
  if (NULL == readyQueue) {
    sigrelse(SIGALRM);
    return;
  }
  // otherwise shuffle accordingly
  else {
    tcb* last = runningQueue;
    readyQueue = insert(readyQueue, runningQueue);
    runningQueue = readyQueue;
    readyQueue = readyQueue->next;
    runningQueue->next = NULL;

    // exit critical section
    sigrelse(SIGALRM);
    swapcontext(last->thread_context, runningQueue->thread_context);
  }
}

void sigalrm_handler(int signal)
{
  // reset signal handling
  sigset(SIGALRM, sigalrm_handler);
  // yield the thread because its time on this earth has expired
  t_yield();
}

void t_init()
{
  for(int i = 0; i < 100; i++){
    tid_map[i] = NULL;
  }
  // create a main thread and start running it
  runningQueue = (tcb*) malloc(sizeof(tcb));
  ucontext_t *tmp;
  tmp = (ucontext_t *) malloc(sizeof(ucontext_t));

  getcontext(tmp);    /* let tmp be the context of main() */
  runningQueue->thread_id = 0;
  runningQueue->thread_priority = 1;
  runningQueue->thread_context = tmp;
  runningQueue->next = NULL;
  readyQueue = NULL;

  mbox_create(&tid_map[0]);
  // start a timer so no thread can run forever
  sigset(SIGALRM, sigalrm_handler);
  //ualarm(interval, 0);
}

int t_create(void (*fct)(int), int id, int pri)
{
  /* sighold(SIGALRM); */
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
  mbox_create(&tid_map[id]);


  if(NULL == readyQueue) {
    readyQueue = tmp;
  }
  else {
    readyQueue = insert(readyQueue, tmp);
  }
  /* sigrelse(SIGALRM); */
  /* free(uc); */
  return 0;
}

//Free a thread control block
void freeTcb(tcb* tmp) {
  free(tmp->thread_context->uc_stack.ss_sp);
  free(tmp->thread_context);
  free(tmp);
}

void t_shutdown() {
  tcb* current = readyQueue;
  tcb* next = current;
  //Loop through both the ready queue and free all threads
  while(NULL != current) {
    next = current->next;
    freeTcb(current);
    current = next;
  }

  //TODO: free tid_map 
  // for(int i = 0; i < 100; i++){
  //   if(tid_map[i] != NULL){
  //       mbox_destroy(&tid_map[i]);
  //   }
  // }

  //Free running queue head
  if (NULL != runningQueue) {
    freeTcb(runningQueue);
  }
}


void t_terminate() {
  // tid_map[runningQueue->thread_id] = NULL;
  //Free the currently running thread from the running queue
  if (runningQueue != NULL) {
    freeTcb(runningQueue);
  }
  //Put the next thread on the ready queue onto the running queue
  runningQueue = readyQueue;
  readyQueue = readyQueue->next;
  runningQueue->next = NULL;
  //Switch to new head

  setcontext(runningQueue->thread_context);
}

int sem_init(sem_t **sp, int sem_count)
{
  *sp = malloc(sizeof(sem_t));
  (*sp)->count = sem_count;
  (*sp)->q = NULL;
}

void sleepTcb(sem_t* s)
{
  tcb* current = s->q;
  if (NULL == s->q) {
    s->q = runningQueue;
    current = s->q;
  }
  else {
    while(current->next != NULL) {
      current = current->next;
    }
    current->next = runningQueue;
  }
  tcb* last = runningQueue;
  runningQueue = readyQueue;
  readyQueue = readyQueue->next;
  runningQueue->next = NULL;
  swapcontext(last->thread_context, runningQueue->thread_context);
}

void sem_wait(sem_t *s)
{
  sighold();
  s->count--;
  if (s->count < 0) {
    sleepTcb(s);
    sigrelse();
  }
  else {
    sigrelse();
  }
}

tcb* wakeTcb(tcb* head) {
  if (NULL == head) {
    return head;
  }
  tcb* current = head;
  head = head->next;
  readyQueue = insert(readyQueue, current);
  return head;
}

void sem_signal(sem_t *s)
{
  sighold();
  s->count++;
  if (s->count <= 0) {
    s->q = wakeTcb(s->q);
    sigrelse();
  }
  sigrelse();
}

void sem_destroy(sem_t **s)
{
  free(*s);
}

// put message in mailbox
void mailboxInsert(mbox* mailbox, struct messageNode* msg) {
  // TODO fix semaphores
  /*sem_wait(mailbox->mbox_sem);*/
  struct messageNode* head = mailbox->msg;
  if (NULL == head) {
    mailbox->msg = msg;
  }
  else {
    struct messageNode* current = mailbox->msg;
    while(current->next != NULL) {
      current = current->next;
    }
    current->next = msg;
  }
  /*sem_signal(mailbox->mbox_sem);*/
}

// dequeue msg from mailbox
struct messageNode* mailboxDequeue(mbox* mailbox) {
  sem_wait(mailbox->mbox_sem);
  struct messageNode* current = mailbox->msg;
  if(NULL != current){
    mailbox->msg = mailbox->msg->next;
  }
  sem_signal(mailbox->mbox_sem);
  return current;
}

// make space for mbox
int mbox_create(mbox **mb) {
  *mb = malloc(sizeof(mbox));
  sem_init(&((*mb)->mbox_sem), SEM_LOCK_INIT);
  return 0;
}

// free memory for mbox
void mbox_destroy(mbox **mb) {
  struct messageNode* current = (*mb)->msg;
  while(NULL != current) {
    free(current->message);
    free(current);
  }
  sem_destroy(&((*mb)->mbox_sem));
  free(*mb);
}

// put message in mailbox
void mbox_deposit(mbox *mb, char *msg, int len) {
  sem_wait(mb->mbox_sem);
  struct messageNode* msgNode = malloc(sizeof(struct messageNode));
  msgNode->len = len;
  msgNode->message = (char*) malloc(len*sizeof(char));
  msgNode->sender = runningQueue->thread_id;
  strcpy(msgNode->message, msg);
  msgNode->next = NULL;
  mailboxInsert(mb, msgNode);
  sem_signal(mb->mbox_sem);
}

// withdraw first message
void mbox_withdraw(mbox *mb, char *msg, int *len) {
  if (NULL == mb->msg) {
    *len = 0;
  }
  struct messageNode* mail = mailboxDequeue(mb);
  if(mail != NULL){
    strncpy(msg, mail->message, mail->len + 1);
    *len = mail->len + 1;
  }
}

// receive message from specific thread
void mbox_withdraw_by_sender(mbox *mb, char *msg, int *len, int *sender) {
  if (NULL == mb->msg) {
    *len = 0;
    *sender = 0;
    return;
  }
  if (*sender  == 0) {
    struct messageNode* msgNode = mailboxDequeue(mb);
    *sender = msgNode->sender;
    strncpy(msg, msgNode->message, strlen(msgNode->message));
    *len = msgNode->len;
    return;
  }
  struct messageNode* current = mb->msg;
  sem_wait(mb->mbox_sem);
  if (mb->msg->sender == *sender) {
    strncpy(msg, current->message, strlen(current->message));
    *len = current->len;

    mb->msg = mb->msg->next;
    struct messageNode* toDelete = current;
    free(toDelete->message);
    free(toDelete);
  }
  else {
    while (current->next != NULL) {
      if (current->next->sender == *sender) {
        strncpy(msg, current->next->message, strlen(current->next->message));
        *len = current->next->len;

        struct messageNode* toDelete = current->next;
        free(toDelete->message);
        free(toDelete);

        current->next = current->next->next;
        break;
      }
      else {
        current = current->next;
      }
    }
  }
  sem_signal(mb->mbox_sem);
}

// send message to thread with tid
void send(int tid, char *msg, int len) {
  if(tid_map[tid] != NULL){
    mbox_deposit(tid_map[tid], msg, len);
  }
}

// receive message sent by tid, store in msg
void receive(int *tid, char *msg, int *len) {
  *len = 0;
  if (0 == *tid) {
    mbox_withdraw_by_sender(tid_map[runningQueue->thread_id], msg, len, tid);
    /*t_yield();*/
  }
  else {
    mbox_withdraw_by_sender(tid_map[runningQueue->thread_id], msg, len, tid);
    /*t_yield();*/
  }
}

