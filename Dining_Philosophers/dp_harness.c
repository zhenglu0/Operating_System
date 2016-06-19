#include <sys/time.h>
/* optarg includes */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include "dp.h"

int chopsticksCount = 3;
int philosophersCount = 3;
int eatTime = 1;
int thinkTime = 3;

int process_args(int argc, char **argv)
{
  int c;
     
  while ((c = getopt (argc, argv, "p:c:e:t:h")) != -1)
    switch (c)
    {
      case 'p':
        philosophersCount = atoi(optarg);
        break;
      case 'c':
        chopsticksCount = atoi(optarg);
        break;
      case 'e':
        eatTime = atoi(optarg);
        break;
      case 't':
        thinkTime = atoi(optarg);
        break;
      case '?':
        fprintf (stderr,
           "Unknown option character `\\x%x'.\n",
           optopt);
        return 1;
      case 'h':
      default:
        printf("-c <count> : chopstick count, default=%d\n",chopsticksCount);
        printf("-p <count> : philosopher count, default=%d\n",philosophersCount);
        printf("-e <time>  : eat time (seconds), default=%d\n",eatTime);
        printf("-t <time>  : think time (seconds), default=%d\n",thinkTime);
        abort ();
    }
  printf("Starting %d philosophers with %d chopsticks\n",chopsticksCount,philosophersCount);
  return 0;
}

void *doWork(void *t)
{
  philosopher_t *me = (philosopher_t*)t;
  philosophize(me);
  printf("done\n");
  pthread_exit(0);
}

void create_philosophers()
{
  philosopher_t philosophers[philosophersCount];
  pthread_t threads[philosophersCount];
  phil_state_t states[philosophersCount];
  sem_t sticks[chopsticksCount];
  
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  int rc;
  long t;
  void *status;
  int i;
  for (i=0; i<chopsticksCount; i++) 
  {
    sem_init(&sticks[i], 0, 1); 
  }
  for (i=0; i<philosophersCount; i++) 
  {
    states[i] = THINKING;
  }
  for (i=0; i<philosophersCount; i++)
  {
    philosophers[i].sticks = &sticks[0];
    philosophers[i].states = &states[0]; 
    philosophers[i].index = i;
    philosophers[i].foodCount = 1;
    philosophers[i].stickCount = chopsticksCount;
    philosophers[i].philCount = philosophersCount;
    rc = pthread_create(&threads[i],&attr, doWork, (void*) (&(philosophers[i])));
    if (rc) {
       fprintf(stderr,"ERROR; return code from pthread_create() is %d\n", rc);
       exit(-1);
    }
  }
    
  pthread_attr_destroy(&attr);
  for(t=0; t<philosophersCount; t++) {
    rc = pthread_join(threads[t], &status);
    if (rc) {
       printf("ERROR; return code from pthread_join() is %d\n", rc);
       exit(-1);
    }
    printf("Main: completed join with thread %ld having a status of %ld\n",t,(long)status);
  }
}

int main(int argc, char **argv)
{
  if (process_args(argc, argv)!=0) exit(1);
  create_philosophers();
  return 0;
}
