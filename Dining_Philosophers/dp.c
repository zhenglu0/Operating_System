#include "dp.h"

int philosophersCounts = 3;
int semStatus = 0;
static pthread_mutex_t cs_mutex = PTHREAD_MUTEX_INITIALIZER;

int putdownLeftStick(philosopher_t *p)
{
  printf("#%d put down left stick\n",p->index);
  sem_post(&(p->sticks[p->index]));
  return 0;
}

int putdownRightStick(philosopher_t *p)
{
  printf("#%d put down right stick\n",p->index);
  sem_post(&(p->sticks[((p->index)+1)%philosophersCounts]));
  return 0;
}

int pickupRightStick(philosopher_t *p)
{
  //pthread_mutex_lock( &cs_mutex );
  printf("#%d picking up right stick\n",p->index);
  //sem_getvalue(&(p->sticks[((p->index)+1)%philosophersCounts]),&semStatus);
  //printf("#%d right stick = %d \n",p->index,semStatus);
  /*if(semStatus == 0 && (p->index) == (philosophersCounts-1))
	{
	printf("#%d are forced to put down left stick for potential deck lock problems\n",p->index);
	sem_post(&(p->sticks[p->index]));		
	}
  else	{
	sem_wait(&(p->sticks[((p->index)+1)%philosophersCounts]));
	printf("#%d got right stick\n",p->index);	 	  
	}*/
  sem_wait(&(p->sticks[((p->index)+1)%philosophersCounts]));
  printf("#%d got right stick\n",p->index);	
  //pthread_mutex_unlock( &cs_mutex );
  return 0;
}

int pickupLeftStick(philosopher_t *p)
{
  printf("#%d picking up left stick\n",p->index);
  /*sem_getvalue(&(p->sticks[p->index]),&semStatus);
  printf("#%d left stick = %d \n",p->index,semStatus);*/
  sem_wait(&(p->sticks[p->index]));
  printf("#%d got left stick\n",p->index);
  return 0;
}

void eat(philosopher_t *p)
{
  p->foodCount--;
  printf("#%d eating\n",p->index);
  sleep(1);
}

void think(philosopher_t *p)
{
  p->thinkCount--;
  printf("#%d thinking\n",p->index);
  sleep(3);
}

int philosophize(philosopher_t *p)
{
  while (p->foodCount > 0)
  {
    pthread_mutex_lock( &cs_mutex );
    pickupLeftStick(p);
    pickupRightStick(p);
    pthread_mutex_unlock( &cs_mutex );
    eat(p);
    putdownLeftStick(p);
    putdownRightStick(p);
    think(p);
    
  }
  printf("#%d done\n",p->index);
  pthread_exit(0);
}
