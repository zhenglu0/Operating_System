#include "task.h"
#include "timer.h"
// test code includes
#include "main_tester.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <ucontext.h>
#include <unistd.h>
// forward declarations
void switch_task();
void exit_task();
void timer_init (u32int frequency);
void setup_children();
void fix_context(ucontext_t *u, int id,void (*func)());
void idle(int param);
void bubblesort(task_t *readyqueue);
void output(task_t *head);

// The currently running task.
static task_t *current_task;
// The start of the task linked list.
static task_t *ready_queue;
// the start of the current queue, it is used to hold all the elements
static task_t *current_queue;
// The start of the sleep task linked list.
static task_t *sleep_queue;
// Shortcut to the idle task.
static task_t *idle_task;

// BEGIN Code to emulate our kernel data structures
// The current tick count
int tick;
// Shortcut for soft blocking interrupts
volatile int intr_blocked = 0;
// The list of child tasks
task_t *children = 0;
// The list of ucontexts associated with each child task
ucontext_t *contexts;
const int stack_sz = 1024*64;	// thread stack is 64 KB
int max_children = 0;
//processor execution time for the ith instance of this processor
int time[10] = {1,2,3,4,5,6,7,8,9,10};
//constant weighting factor
float a = 0.8;  
task_t *head;
	
// END code

int main()
	{
	printf("!!!!!!!!!!main start!!!!!!!!!!\n");
	setup_children(10);
	timer_init(100);
	bubblesort(current_queue);
	current_task = head->next;
	switch_task();
	printf("!!!!!!!!!!main done!!!!!!!!!!\n");
	idle(0);
	return 0;
	}

void switch_task() 
	{
	printf("!!!!!!!!!!executing swith_task!!!!!!!!!!\n");
	// insert or update scheduling code here
	intr_blocked=1;
	task_t *tmp, *old_task,*p,*q,*tail,*t;
	if (current_queue) 
		{
		// update the tick count
		current_task->ticks++;
		printf("cq=%d ",current_task->id);
		// add the current task to the end of the ready queue
		printf("rq=");
		ready_queue = current_queue->next;
		for(tmp=ready_queue;tmp->next != 0; tmp=tmp->next) 
			{
			printf("%d ",tmp->id);
			}
		printf("%d \n",tmp->id);
		//adjusting the ready queue
		current_queue = current_queue ->next;
		tmp->next = current_task;
		current_task->next = 0;
		old_task = current_task;
		//after the current task, use the bubble sort to sort the task	
		printf("Before Sort ");
		for(tmp=current_queue;tmp->next != 0; tmp=tmp->next) 
			{
			printf("%d ",tmp->id);
			}
		printf("%d \n",tmp->id);		
		head->next = current_queue;
		tail=0;
		while(head->next!=tail)
			{
			p=head;
			q=p->next;
				while(q->next!=tail)
				{
				if((p->next->s) > (q->next->s))
					{   
					t=q->next;
					p->next=q->next;
					q->next=q->next->next;
					p->next->next=q;
					q=t;
					}
					p=p->next;
					q=q->next;
				}
			tail=q;
			}
		printf("After  Sort ");
		for(tmp=head->next;tmp->next != 0; tmp=tmp->next) 
			{
			printf("%d ",tmp->id);
			}
		printf("%d \n",tmp->id);	
		
		// set the current task to be the head of the ready queue
		current_task = head->next;
		// pop off the head task in the ready queue
		current_queue = head->next;
		printf("Switching from %d [%d ticks] to %d\n",old_task->id,old_task->ticks,current_task->id);
		intr_blocked=0;
		printf("Before Calculation ");
		printf("old_task[%d].S = %f \n",old_task->id,old_task->s);		
		printf("After  Calculation ");
		//calulating the predicted value
		if((old_task->ticks >= 2)&&(old_task->ticks <= 10))
			{
				//(old_task->s)++;
			old_task->s = a*time[(old_task->ticks)-2] + (1-a)*(old_task->s); 
			} 
		else if(old_task->ticks >= 11) 
			{
			//after the 10 times
			old_task->s = a*10 + (1-a)*old_task->s; 
			}   
		printf("old_task[%d].S = %f \n",old_task->id,old_task->s);
		printf("old_task->id = %d, current_task->id = %d \n",old_task->id,current_task->id);
		Swapcontext(&contexts[old_task->id],&contexts[current_task->id]);
		
		} 
	else 
		{
		printf("switching to idle task\n");
		current_task = idle_task;
		}
	intr_blocked=0;
}

void check_sleep_queue() 
	{
	// insert/update code here
	}
	
//exit task
void exit_task(int id) 
	{
	//task_t *temp;
	printf("**************task id = %d is finished************** \n",id);
	//delete the task from the queue
	//temp = current_queue;
	current_task = current_queue->next;
	current_queue = current_queue->next;
	//temp->next = 0;	
	// insert/update code here
	}
// using bubble sort to sort the ready queue	
void bubblesort(task_t *readyqueue)
	{
	head->next = readyqueue;
	task_t *p,*q,*tail,*temp;
	tail=0;
	while(head->next!=tail)
		{
		p=head;
		q=p->next;
			while(q->next!=tail)
			{
			if((p->next->s) > (q->next->s))
				{   
				temp=q->next;
				p->next=q->next;
				q->next=q->next->next;
				p->next->next=q;
				q=temp;
				}
				p=p->next;
				q=q->next;
			}
		tail=q;
		}
	}
//print the linklist	
void output(task_t *head)
	{
	task_t *p;
	p=head->next;
	while(p)
		{
		printf("Ready queue after sort by s %d \n",p->id);
		p=p->next;
		}
	}
	
// BEGIN code to emulate our kernel
//  You shouldn't need to change anything past here, but you can
void timer_callback() 
	{
	fprintf(stderr,"ticks %d\n",tick);
	if (!intr_blocked) 
		{
		tick++;
		check_sleep_queue();
		switch_task();
	
		//printf("timer_callback()\n");
		}
	}

void timer_init (u32int frequency) 
	{
	struct itimerval	tv;
	struct sigaction	act;
	int			sec, usec;
	printf("initializing timer %d\n",frequency);
	if ( frequency > 0 ) 
		{
		act.sa_handler = timer_callback;		// init SIGVTALRM handler
		act.sa_flags = 0;
		if (sigemptyset(&act.sa_mask) || sigaction(SIGVTALRM, &act, NULL)) 
			{
			perror("Gurk!!! Failed to init SIGVTALRM handler");
			exit(1);
			}
		sec = frequency/1000;
		usec = 1000*(frequency - 1000*sec);
		printf("setting sec=%d, usec=%d\n",sec, usec);
		tv.it_interval.tv_sec = sec;
		tv.it_interval.tv_usec = usec;
		tv.it_value = tv.it_interval;
		if (setitimer(ITIMER_VIRTUAL, &tv, NULL) == -1) 
			{
		perror("Gurk!!! Failed to init ITIMER_VIRTUAL");
		exit(1);
			}
		}
	}

void do_work(int param);

void setup_children(int count) 
	{
	int i;
	task_t *tmp;
	// add one for idle task
	count++; 
	max_children=count;
	// allocate children
	children = (task_t *)malloc(count * sizeof(task_t));
	contexts = (ucontext_t *)malloc(count * sizeof(ucontext_t));
	//initalising head
	head = (task_t *)malloc(sizeof(task_t));
	// setup child tasks/contexts
	for (i=1; i<count; i++) 
		{
		tmp = &(children[i]);
		if (i < count - 1)
		tmp->next = &(children[i+1]);
		children[i].id = i;
		children[i].s = 0;
		Getcontext(&(contexts[i]));
		fix_context(&(contexts[i]), i, &do_work);
		}
	tmp->next = 0;
	// setup idle task
	Getcontext(&contexts[0]);
	fix_context(&contexts[0], 0, &idle);
	children[0].id=0;
	children[0].next =0;
	idle_task = &(children[0]);
	current_task = 0;
	current_queue = &children[1];
	sleep_queue = 0;
	//set the child value
	float temp = 0;
	//assign testing value
	for (i=1; i<=count/2; i++) 
		{
		temp = a*(i-1) + (1-a)*temp;
		printf("i = %d temp = %f\n",i,temp);
		//assign testing value
		children[i].s = temp;
		children[i].ticks = i;
		children[count-i].s=temp;	
		children[count-i].ticks = i;
		//printf("count = %d \n",count);
		//printf("count - i = %d \n",count-i);
		}
	}

void idle(int param) 
	{
	while(1) 
		{ 
		//printf("!");
		}
	}

void do_work(int param) 
	{
	int id = param;
	printf("starting child %d\n",id);
	//printf("children[%d].ticks*%d\n",id,id);
	while(1) { 
		//printf("running %d\n",id);
    if (children[id].ticks*id >= 100) 
		{
		fprintf(stderr,"--------------------------child %d done--------------------------\n",id);
		exit_task(id);
		return;
		}
	}
	}

void fix_context(ucontext_t *u, int id,void (*func)()) 
	{
	u->uc_link = &contexts[0];
	u->uc_stack.ss_sp = malloc(stack_sz );
	u->uc_stack.ss_size = stack_sz;
	u->uc_stack.ss_flags = 0; 	    
	Makecontext(u,func,id);
	}

// END Code
