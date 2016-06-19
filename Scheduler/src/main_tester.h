#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef MAIN_TESTER_H
#define MAIN_TESTER_H

// --| ucontext wrappers |--
static inline int
Getcontext( ucontext_t *ucp ) {
  int	rc;
  rc = getcontext(ucp);
  if (rc != 0) {
    perror( "*** Bad getcontext rc. " );
    exit(1);
  }
  return rc;
}

static inline void
Makecontext(ucontext_t *ucp, void (*func)(), int arg0) {
  makecontext( ucp, func, 1, arg0 );
}

static inline int
Swapcontext ( ucontext_t *oucp, ucontext_t *ucp ) {
  int	rc;
  rc = swapcontext( oucp, ucp );
  if (rc != 0) {
    perror( "*** Bad swapcontext rc. " );
    exit(1);
  }
  return rc;
}

#endif /* MAIN_TESTER_H */
