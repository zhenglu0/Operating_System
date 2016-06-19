#ifndef _DP_H_
#define _DP_H_

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

typedef enum phil_state
{
  THINKING = 0,
  HUNGRY = 1,
  EATING = 2
} phil_state_t;

typedef struct philosopher
{
  int index;
  phil_state_t *states;
  sem_t *sticks;
  int philCount;
  int stickCount;
  int foodCount;
  int thinkCount;

} philosopher_t;

int philosophize(philosopher_t *philosopher);

#endif  /* _DP_H_ */
