/*
 * file: preemptive.h
 *
 * this is the include file for the cooperative multithreading
 * package.  It is to be compiled by SDCC and targets the EdSim51 as
 * the target architecture.
 *
 * CS 3423 Fall 2018
 */

#ifndef __PREEMPTIVE_H__
#define __PREEMPTIVE_H__

#define MAXTHREADS 4  /* not including the scheduler */
/* the scheduler does not take up a thread of its own */


// _ concatenate s
#define CNAME(s) _ ## s
#define conca(s) s ## $
// set s = n, n = 0x01 
//  mov	r0,#CNAME(s) 
//  mov	@r0,#n
#define SemaphoreCreate(s, n) {s=n;}
#define SemaphoreCreates(s, n) \
{ __asm   \
  mov CNAME(s), n  ;; this will go to get the data from the address n\
  __endasm;   \
}

// do busy-wait on semaphore s
#define SemaphoreSignal(s) \
{ __asm   \
  inc CNAME(s)\
  __endasm;   \
}

// signal a semaphore s
//#define SemaphoreWait(s) SemaphoreWaitBody(s, conca(__COUNTER__))
#define SemaphoreWait(s)\
{\
    SemaphoreWaitBody(s, __COUNTER__);\
}

#define SemaphoreWaitBody(s, label) \
{ __asm \
conca(label): mov a, CNAME(s);; top of while-loop    ;; read value of _s into accumulator (where s is the semaphore) \\
  jz conca(label) ;; use conditional jump(s) to jump back to label if accumulator <= 0 \
  jb ACC.7, conca(label) ;; fall-through to drop out of while-loop \
  dec CNAME(s) \
__endasm; \
}
typedef char ThreadID;
typedef void (*FunctionPtr)(void);

void ThreadCreate(FunctionPtr);
void ThreadYield(void);
void ThreadExit(void);


#endif // __PREEMPTIVE_H__
