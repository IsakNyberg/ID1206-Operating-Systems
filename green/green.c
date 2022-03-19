//#define _XOPEN_SOURCE 600  // this fixes some mac issues
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include <stdio.h>
#include "green.h"
#include <signal.h>
#include <sys/time.h>

#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096
#define PERIOD 100

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, NULL, FALSE};
static green_t *running = &main_green;
static sigset_t block;

static void init() __attribute__((constructor));
void timer_handler(int);


static struct green_t *q_first;
static struct green_t *q_last;
void q_append(struct green_t *thread){
    if (thread == NULL) {
        printf("added null to queue");
    }
    if (q_first == NULL) {
        q_first = thread;
        q_last = thread;
        return;
    }
    q_last->next = thread;
    q_last = thread;
    q_last->next = NULL;
}

struct green_t *q_pop(){
    if (q_first == NULL){
        printf("Tried popping empty queue\n");
        return NULL;
    }
    struct green_t *res = q_first;
    if (q_first == q_last){
        q_last = NULL;
    }
    q_first = q_first->next;
    res->next = NULL;
    if (res->zombie){
        printf("Zombie in queue\n");
        return q_pop();
    }
    return res;
}

void timer_handler(int sig) {
    green_t *susp = running;
    q_append(susp);
    struct green_t *next = q_pop();
    running = next;
    swapcontext(susp->context, next->context);
}

void init() {
    sigemptyset(&block);
    sigaddset(&block, SIGVTALRM);

    struct sigaction act = {0};
    struct timeval interval;
    struct itimerval period;

    act.sa_handler = timer_handler;
    assert(sigaction(SIGVTALRM, &act, NULL) == 0);

    interval.tv_sec = 0;
    interval.tv_usec = PERIOD;
    period.it_interval = interval;
    period.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &period, NULL);
    getcontext(&main_cntx);
}

void green_thread(){
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    struct green_t *this = running;

    void *resut = (*this->fun)(this->arg);
    // place waiting thread in ready q if necessary
    sigprocmask(SIG_BLOCK, &block, NULL);
    if (this->join != NULL){
        q_append(this->join);
    }
    // save result from execution
    this->retval = resut;
    // zombie 
    this->zombie = TRUE;

    // find next thread to run
    struct green_t *next = q_pop();
    running = next;
    //

    setcontext(next->context);
}


int green_create(green_t *new, void *(*fun)(void*), void *arg) {
    sigprocmask(SIG_BLOCK, &block, NULL);

    ucontext_t *cntx = (ucontext_t*)malloc(sizeof(ucontext_t));
    getcontext(cntx);
    void *stack = malloc(STACK_SIZE);
    cntx->uc_stack.ss_sp = stack;
    cntx->uc_stack.ss_size = STACK_SIZE;
    makecontext(cntx, green_thread, 0);
    new->context = cntx;
    new->fun = fun;
    new->arg = arg;
    new->next = NULL;
    new->join = NULL;
    new->retval = NULL;
    new->zombie = FALSE;
    // add new to the ready queue
    q_append(new);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_yield() {
    sigprocmask(SIG_BLOCK, &block, NULL);
    struct green_t *susp = running;
    // add susp to ready queue

    q_append(susp);
    // select next thread for execution
    green_t *next = q_pop();

    running = next;

    swapcontext(susp->context, next->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);  // ???
    return 0;
}


int green_join(green_t *thread, void **res) {
    sigprocmask(SIG_BLOCK, &block, NULL);
    if (thread->zombie == FALSE){
        struct green_t *susp = running;
        // add as joining thread
        thread->join = susp;
        // select next thread for execution

        struct green_t *next = q_pop();
        running = next;
        swapcontext(susp->context, next->context);
        sigprocmask(SIG_UNBLOCK, &block, NULL);
    }
    // collect result
    if (thread->retval != NULL) {
        res = thread->retval;
    }
    // free context
    free(thread->context->uc_stack.ss_sp);
    free(thread->context);
    return 0;
}


void green_cond_init(green_cond_t* cond){
    sigprocmask(SIG_BLOCK, &block, NULL);
    cond->first = NULL;
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

void green_cond_wait(green_cond_t* cond, green_mutex_t *mutex){
    sigprocmask(SIG_BLOCK, &block, NULL);
    struct green_t *susp = running;
    
    if (cond->first == NULL) {
        cond->first = susp;
    } else {
        struct green_t* temp = cond->first;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = susp;
    }
    if (mutex != NULL) {
        while (mutex->first != NULL) {  // add eveything to ready queu
            q_append(mutex->first);
            mutex->first = mutex->first->next;
        }
        mutex->taken = FALSE;
    }
    struct green_t *next = q_pop();
    running = next;
    swapcontext(susp->context, next->context);
        
    if (mutex != NULL) {
        while(mutex->taken == TRUE) {
            struct green_t *susp = running;
            struct green_t *next = q_pop();
            running = next;
            swapcontext(susp->context, next->context);
        } //else {
        mutex->taken = TRUE;
        //}
    }
    
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

void green_cond_signal(green_cond_t* cond){
    sigprocmask(SIG_BLOCK, &block, NULL);
    if (cond->first != NULL) {
        q_append(cond->first);
        cond->first = cond->first->next;
    }
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}



int green_mutex_init(green_mutex_t *mutex) {
    mutex->taken = FALSE;
    mutex->first = NULL;
    return 0;
}

int green_mutex_lock(green_mutex_t *mutex) {
    sigprocmask(SIG_BLOCK, &block, NULL);
    struct green_t *susp = running;
    while (mutex->taken == TRUE){
        // somebody else already has the mutex
        // put thread on mutex queue
        struct green_t* temp = mutex->first;
        if (temp == NULL) {  // empty list
            mutex->first = susp;
        } else {  // put in last place
            while (temp->next != NULL) {
                temp = temp->next;
            }
            temp->next = susp;
        }
        
        // pick next thread to schedule
        green_t *next = q_pop();
        running = next;
        swapcontext(susp->context, next->context);
    } //else {
        // take the lock
    mutex->taken = TRUE;
    //}
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_mutex_unlock(green_mutex_t *mutex) {
    sigprocmask(SIG_BLOCK, &block, NULL);
    while (mutex->first != NULL) {  // add eveything to ready queu
        q_append(mutex->first);
        mutex->first = mutex->first->next;
    }
    mutex->taken = FALSE;
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

