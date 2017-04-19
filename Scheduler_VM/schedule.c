

/* schedule.c
 * This file contains the primary logic for the 
 * scheduler.
 */
#include "schedule.h"
#include "macros.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#define a 0.5
#define NEWTASKSLICE (NS_TO_JIFFIES(100000000))

/* Local Globals
 * rq - This is a pointer to the runqueue that the scheduler uses.
 * current - A pointer to the current running task.
 */
struct runqueue *rq;
struct task_struct *current;

/* External Globals
 * jiffies - A discrete unit of time used for scheduling.
 *			 There are HZ jiffies in a second, (HZ is 
 *			 declared in macros.h), and is usually
 *			 1 or 10 milliseconds.
 */
extern long long jiffies;
extern struct task_struct *idle;

/*-----------------Initilization/Shutdown Code-------------------*/
/* This code is not used by the scheduler, but by the virtual machine
 * to setup and destroy the scheduler cleanly.
 */
 
/* initscheduler
 * Sets up and allocates memory for the scheduler, as well
 * as sets initial values. This function should also
 * set the initial effective priority for the "seed" task 
 * and enqueu it in the scheduler.
 * INPUT:
 * newrq - A pointer to an allocated rq to assign to your
 *			local rq.
 * seedTask - A pointer to a task to seed the scheduler and start
 * the simulation.
 */
void initschedule(struct runqueue *newrq, struct task_struct *seedTask)
{
  seedTask->next = seedTask->prev = seedTask;
  newrq->head = seedTask;
  newrq->nr_running++;
	
}

/* killschedule
 * This function should free any memory that 
 * was allocated when setting up the runqueu.
 * It SHOULD NOT free the runqueue itself.
 */
void killschedule()
{
  return;
}


void print_rq () {
  struct task_struct *curr;
	
  printf("Rq: \n");
  curr = rq->head;
  if (curr)
    printf("%p", curr);
  while(curr->next != rq->head) {
    curr = curr->next;
    printf(", %p", curr);
  };
  printf("\n");
}

/*-------------Scheduler Code Goes Below------------*/
/* This is the beginning of the actual scheduling logic */

/* schedule
 * Gets the next task in the queue
 */
void schedule()
{
  static struct task_struct *nxt = NULL;
  struct task_struct *curr,*todo;
  int i;
  long minexp=0;
  long maxwait=0;
  long last_duration=0;
  long min_goodness;
  //	printf("In schedule\n");
  // 	print_rq();
  current->need_reschedule = 0; /* Always make sure to reset that, in case *
				 * we entered the scheduler because current*
				 * had requested so by setting this flag   */
	
  if (rq->nr_running == 1) {
    context_switch(rq->head);
    nxt = rq->head->next;
  }
  else {	
    last_duration=sched_clock()-current->last_cpu_taken;
    current->last_rq_enter=sched_clock();
    current->exp_burst=(a*current->exp_burst+last_duration)/(1+a);
    //find minexp for next choice
    minexp=rq->head->next->exp_burst;
    for(i=0;i<rq->nr_running;i++){// vrisko to megisto xrono anamonis kai min exp_bust
      if (nxt == rq->head)    //na min valei tin init
	nxt = nxt->next;
      curr = nxt;
      nxt = nxt->next;
      if(minexp>(curr->exp_burst)){
	minexp=curr->exp_burst;
      }
      if(maxwait<sched_clock()-curr->last_rq_enter){
	maxwait=sched_clock()-curr->last_rq_enter;
      }
    }
    min_goodness=((1+rq->head->next->exp_burst)/(minexp+1))*((maxwait+1)/1+sched_clock()-rq->head->next->last_rq_enter);
  
      todo=rq->head->next;
    for(i=0;i<rq->nr_running;i++){// vrisko to megisto xrono anamonis kai min exp_bust
      if (nxt == rq->head)    //na min valei tin init
	nxt = nxt->next;
      curr = nxt;
      nxt = nxt->next;
      curr->goodness=((1+curr->exp_burst)/(minexp+1))*((maxwait+1)/1+sched_clock()-curr->last_rq_enter);
      if((curr->goodness<min_goodness)&&(curr!=rq->head)){
	min_goodness=curr->goodness;
	todo=curr;
 
      }
	
     
    }
    //last_duration einai otan pairnei cpu
    /* processes */

    if(current!=todo)
      todo->last_cpu_taken=sched_clock();

    if(todo->next==rq->head)
      nxt=todo->next->next;
    else
      nxt=todo->next;
    if(todo==rq->head){
      
      printf("####Mpike i init mesa\n");
    }
    
      context_switch(todo);
    
    
  }
 
}


/* sched_fork
 * Sets up schedule info for a newly forked task
 */
void  sched_fork(struct task_struct *p)
{
  p->last_cpu_taken=0;
  p->exp_burst=0;
  p->last_rq_enter=sched_clock();
  p->time_slice = 100;
}

/* scheduler_tick
 * Updates information and priority
 * for the task that is currently running.
 */
void scheduler_tick(struct task_struct *p)
{
  schedule();
}

/* wake_up_new_task
 * Prepares information for a task
 * that is waking up for the first time
 * (being created).
 */
void wake_up_new_task(struct task_struct *p)
{	
  p->next = rq->head->next;
  p->prev = rq->head;
  p->next->prev = p;
  p->prev->next = p;
	
  rq->nr_running++;
}

/* activate_task
 * Activates a task that is being woken-up
 * from sleeping.
 */
void activate_task(struct task_struct *p)
{
  
    p->next = rq->head->next;
    p->prev = rq->head;
    p->next->prev = p;
    p->prev->next = p;
    p->last_rq_enter=sched_clock();
    rq->nr_running++;
  
}

/* deactivate_task
 * Removes a running task from the scheduler to
 * put it to sleep.
 */
void deactivate_task(struct task_struct *p)
{ 
  
    p->prev->next = p->next;
    p->next->prev = p->prev;
    p->next = p->prev = NULL; /* Make sure to set them to NULL *
			     * next is checked in cpu.c      */
  
    rq->nr_running--;
 

}
