#include <8051.h>
#include "preemptive.h"
/*
* @@@ [2 pts] declare the static globals here using
* _idata __at (address) type name; syntax
* manually allocate the addresses of these variables, for
* - saved stack pointers (MAXTHREADS)
* - current thread ID
* - a bitmap for which thread ID is a valid thread;
* maybe also a count, but strictly speaking not necessary
* - plus any temporaries that you need.
*/
__idata __at (0x30) char savedSP[4];    //(0x36~0x39)= {0x00, 0x00, 0x00, 0x00}
__idata __at (0x34) char threadBitmap;  //4bits ; char = 1byte
__idata __at (0x35) char currentThread; //2bits
__idata __at (0x36) char threadCount;
__idata __at (0x37) char callerThreadSP;
__idata __at (0x38) char tid;
__idata __at (0x39) char bitmask;
__idata __at (0x3A) char bitlook;
//__idata __at (0x3B) char semThreads;
__idata __at (0x3C) char wakeQueueFlag;
__idata __at (0x3D) char wakeQueue[2];

// static: means that the variable can only be used in the current .c file 
//static __idata __at (0x20) char sleep0 = 0;
//static __idata __at (0x21) char sleeptime0 = 0;
__idata __at (0x20) char time;
__idata __at (0x21) char n;
__idata __at (0x22) char sleep0;
__idata __at (0x23) char sleeptime0;
__idata __at (0x24) char sleep1;
__idata __at (0x25) char sleeptime1;
__idata __at (0x26) char sleep2;
__idata __at (0x27) char sleeptime2;
__idata __at (0x28) char sleep3;
__idata __at (0x29) char sleeptime3;
/*
* @@@ [8 pts]
* define a macro for saving the context of the current thread by
* 1) push ACC, B register, Data pointer registers (DPL, DPH), PSW
* 2) save SP into the saved Stack Pointers array
* as indexed by the current thread ID.
* Note that 1) should be written in assembly,
* while 2) can be written in either assembly or C
*/
/*
1. push ACC, B, DPTR, PSW onto stack
as part of pushing PSW, also saves register bank
• save stack pointer for the current thread
• Defined as C macros written in inlined assembly
*/
/*testparking.c:58: else spots[1] = currentThread;
	mov	r0,#(_spots + 0x0001)
	mov	@r0,_currentThread
    */
#define SAVESTATE \
{    __asm   \
        push A  \
        push B  \
        push DPL   \
        push DPH    \
        push PSW    \
    __endasm;   \
    savedSP[currentThread] = SP;\
}
/*
* @@@ [8 pts]
* define a macro for restoring the context of the current thread by
* essentially doing the reverse of SAVESTATE:
* 1) assign SP to the saved SP from the saved stack pointer array
* 2) pop the registers PSW, data pointer registers, B reg, and ACC
* Again, popping must be done in assembly but restoring SP can be
* done in either C or assembly.
*/
#define RESTORESTATE \
{    \
    SP = savedSP[currentThread];\
    __asm   \
        pop PSW \
        pop DPH \
        pop DPL \
        pop B   \
        pop A   \
    __endasm;   \
}

//void delay(unsigned char n){
void delay(void)__critical{  
    if(currentThread == 0x00){  
        sleep0 = 1;  
        sleeptime0 = time;
    }                                                     
    else if(currentThread == 0x01){
        sleep1 = 1;             
        sleeptime1 = 8;    
    }
    else if(currentThread == 0x02){
        sleep2 = 1;
        //sleeptime2 = time;
        if(sleeptime1 > 1)sleeptime2 = sleeptime1;
        else sleeptime2 = 8;
    }
    else{
        sleep3 = 1;
        if(sleeptime2 > 1)sleeptime3 = sleeptime2;
        else sleeptime3 = 8;
        //sleeptime3 = time;
    }
}
/*
* we declare main() as an extern so we can reference its symbol
* when creating a thread for it.
*/
extern void main(void);

/*
* ISR for Timer0
*/
void myTimer0Handler() {
/*
* use threadYield():
* do round-robin policy for now.
* find the next thread that can run and
* set the current thread ID to it,
* so that it can be restored 
* there should be at least one thread, so this loop
* will always terminate.
*/  
    EA = 0; //disable interrupt(like __critical)
    SAVESTATE;
    //B = R0; //save R0 value in B, before the next two line trash it.
    __asm
        mov b, r0
        mov dpl, r1
    __endasm;
    
    /* update the time*/
    if(time == 255)time = 0;
    else time++;
    
    /* threadYield 1.0 */
    if(currentThread == 0){
        bitlook = threadBitmap >> 1;
        
        bitmask = 0b1110;
        if ((bitlook | bitmask ) != bitmask) currentThread = 0x01;
        else{
            bitmask = 0b1101;
            if ((bitlook | bitmask ) != bitmask) currentThread = 0x02;
            else{
                bitmask = 0b1011;
                if ((bitlook | bitmask ) != bitmask) currentThread = 0x03;
            }
        }   
    }
    else if(currentThread == 1){
        //determine bitlook
        bitlook = threadBitmap >> 2;
        
        bitmask = 0b0001;
        tid = (threadBitmap & bitmask) << 2;
        bitlook = bitlook | tid; 
        
        bitmask = 0b1110;
        if ((bitlook | bitmask ) != bitmask) currentThread = 0x02;
        else{
            bitmask = 0b1101;
            if ((bitlook | bitmask ) != bitmask) currentThread = 0x03;
            else{
                bitmask = 0b1011;
                if ((bitlook | bitmask ) != bitmask) currentThread = 0x00;
            }
        }
    }
    else if(currentThread == 2){
        bitlook = threadBitmap << 6; //14-8
        bitlook = threadBitmap >> 5;
        bitlook = bitlook | (threadBitmap >> 3);
        
        bitmask = 0b1110;
        if ((bitlook | bitmask ) != bitmask) currentThread = 0x03;
        else{
            bitmask = 0b1101;
            if ((bitlook | bitmask ) != bitmask) currentThread = 0x00;
            else{
                bitmask = 0b1011;
                if ((bitlook | bitmask ) != bitmask) currentThread = 0x01;
            }
        }
    }
    else{
        bitlook = threadBitmap << 5;
        bitlook = threadBitmap >> 5;
        
        bitmask = 0b1110;
        if ((bitlook | bitmask ) != bitmask) currentThread = 0x00;
        else{
            bitmask = 0b1101;
            if ((bitlook | bitmask ) != bitmask) currentThread = 0x01;
            else{
                bitmask = 0b1011;
                if ((bitlook | bitmask ) != bitmask) currentThread = 0x02;
            }
        }
    }
    
    /* deal with the wake up thread */
    if(sleep1 == 1) sleeptime1--;
    if(sleep2 == 1) sleeptime2--;
    if(sleep3 == 1) sleeptime3--;
    /* if a thread just finish delay(n), assign it to the currentThread. */
    if(wakeQueueFlag == 0){
        if(sleep1 == 1 && sleeptime1 == 0){
            sleep1 = 0;
            currentThread = 1;
        }
        if(sleep2 == 1 && sleeptime2 == 0){
            sleep2 = 0;
            currentThread = 2;
        }
        if(sleep3 == 1 && sleeptime3 == 0){
            sleep3 = 0;
            currentThread = 3;
        }
        
        /*deal with same wake up time*/
        if(sleep1 == 1 && sleeptime1 == 1 
            && sleep2 == 1 && sleeptime2 == 1){
            wakeQueueFlag = 2;
            wakeQueue[0] = 1;
            wakeQueue[1] = 2;
        }
        if(sleep1 == 1 && sleeptime1 == 1 
            && sleep3 == 1 && sleeptime3 == 1){
            wakeQueueFlag = 2;
            wakeQueue[0] = 1;
            wakeQueue[1] = 3;
        }
        if(sleep2 == 1 && sleeptime2 == 1 
            && sleep3 == 1 && sleeptime3 == 1){
            wakeQueueFlag = 2;
            wakeQueue[0] = 2;
            wakeQueue[1] = 3;    
        }   
    }
    else if(wakeQueueFlag == 2){ //use flag to deal with the worst case
        if(sleeptime1 == 0) sleep1 = 0;
        if(sleeptime2 == 0) sleep2 = 0;
        if(sleeptime3 == 0) sleep3 = 0;
        currentThread = wakeQueue[0]; //at time n wakeup queue0 first
        wakeQueueFlag = 1;
        wakeQueue[0] = 0;
    }
    else{
        currentThread = wakeQueue[1];
        wakeQueueFlag = 0;
        wakeQueue[1] = 0;
    }
    __asm
        mov r0, b
        mov r1, dpl
    __endasm;
    RESTORESTATE;
    EA = 1;
    
    //return from the interrupt, prohibit the compiler to do RET
    __asm
        RETI
    __endasm;
    
}

/*
* Bootstrap is jumped to by the startup code to make the thread for
* main, and restore its context so the thread can run.
*/
void Bootstrap(void) {
	/*
	* initialize data structures for threads (e.g., mask)
	*
	* optional: move the stack pointer to some known location
	* only during bootstrapping. by default, SP is 0x07.
	*/
    bitmask = 0b0000;
	SP = 0x07;
	threadBitmap = 0x00;
	currentThread = 0x00;
    wakeQueueFlag = 0;
    wakeQueue[0] = 0;
    wakeQueue[1] = 0;
	/*
	* create a thread for main; be sure current thread is
	* set to this thread ID, and restore its context,
	* so that it starts running main().
	*/
    // (1) initialize thread mgr vars
    // (2) create thread for main
    //set up Timer 0 to cause preemption. before you create the initial thread 
    TMOD = 0; // timer 0 mode 0
    IE = 0x82; // enable timer 0 interrupt; keep consumer polling
                // EA - ET2 ES ET1 EX1 ET0 EX0
    TR0 = 1; // set bit TR0 to start running timer 0
	time = 0x00;
    //tid = ThreadCreate(main);
    ThreadCreate(main);
    // (3) set thread ID
    //currentThread = tid;
    currentThread = 0;
    // (4) restore
    RESTORESTATE;
}

/*
* ThreadCreate() creates a thread data structure so it is ready
* to be restored (context switched in).
* The function pointer itself should take no argument and should
* return no argument.
*/
//ThreadID ThreadCreate(FunctionPtr fp) __critical{
void ThreadCreate(FunctionPtr fp) __critical{
	/*
	* check to see we have not reached the max #threads.
	* if so, return -1, which is not a valid thread ID.
	*/
    //flag = threadBitmap; //used to see what is going on 
	//i used the semaphore to do the check!
    //if(threadCount >= 4) return -1;
    /**
    * otherwise, find a thread ID that is not in use,
    * and grab it. (can check the bit mask for threads),
    **/
    bitmask = 0b1110;
    if((threadBitmap | bitmask )== bitmask){
        tid = 0x00;         //0x38
        bitmask = 0b0001;  //0x39
        threadBitmap = threadBitmap | bitmask; //0x34
    }
    else{
        bitmask = 0b1101;
        if ((threadBitmap | bitmask ) == bitmask){
            tid = 0x01;
            bitmask = 0b0010;
            threadBitmap = threadBitmap | bitmask;
        }
        else{
            bitmask = 0b1011;
            if ((threadBitmap | bitmask ) == bitmask){
                tid = 0x02;
                bitmask = 0b0100;
                threadBitmap = threadBitmap | bitmask;
            }
            else{
                bitmask = 0b0111;
                if ((threadBitmap | bitmask ) == bitmask){
                    tid = 0x03;
                    bitmask = 0b1000;
                    threadBitmap = threadBitmap | bitmask;
                }
            }
        }
    }
	/** [18 pts] below
	** a. update the bit mask
	**(and increment thread count, if you use a thread count,
	**but it is optional)
	*/
    threadCount++;  //0x36
		
	/**
	**b. calculate the starting stack location for new thread
	**/
    /*
    switch(threadCount){
        case 1:
            startStackLocation = 0x3F;
        case 2:
            startStackLocation = 0x4F;
        case 3:
            startStackLocation = 0x5F;
        case 4:
            startStackLocation = 0x6F;
    }*/
    callerThreadSP = SP; //0x37
    SP = 0x3F;
    SP += (tid*16);
	
	/**
	**c. save the current SP in a temporary
	**set SP to the starting location for the new thread
	**/
	
	/**
	d. push the return address fp (2-byte parameter to
	ThreadCreate) onto stack so it can be the return
	address to resume the thread. Note that in SDCC
	convention, 2-byte ptr is passed in DPTR. but
	push instruction can only push it as two separate
	registers, DPL and DPH.
	**/
    __asm
    push DPL
    push DPH
    __endasm;
	
	/**
	e. we want to initialize the registers to 0, so we
	assign a register to 0 and push it four times
	for ACC, B, DPL, DPH. Note: push #0 will not work
	because push takes only direct address as its operand,
	but it does not take an immediate (literal) operand.
	**/
    __asm
    mov DPL, #0x00  //using r0 may collide
    push DPL
    push DPL
    push DPL
    push DPL
    __endasm;
    /**
    f. finally, we need to push PSW (processor status word)
    register, which consist of bits
    CY AC F0 RS1 RS0 OV UD P
    all bits can be initialized to zero, except <RS1:RS0>
    which selects the register bank.
    Thread 0 uses bank 0, Thread 1 uses bank 1, etc.
    Setting the bits to 00B, 01B, 10B, 11B will select
    the register bank so no need to push/pop registers
    R0-R7. So, set PSW to
    00000000B for thread 0, 00001000B for thread 1,
    00010000B for thread 2, 00011000B for thread 3.
    **/
    PSW = (tid << 3);
    __asm
        push PSW
    __endasm;
    PSW = PSW >> 3;
    
    /**
    g. write the current stack pointer to the saved stack
    pointer array for this newly created thread ID
    h. set SP to the saved SP in step c.
    i. finally, return the newly created thread ID.
    **/
    savedSP[tid] = SP;
    SP = callerThreadSP;

    //return tid;
}

/*
* this is called by a running thread to yield control to another
* thread. ThreadYield() saves the context of the current
* running thread, picks another thread (and set the current thread
* ID to it), if any, and then restores its state.
*/
void ThreadYield( void ) __critical{
/*
* @@@ [8 pts] do round-robin policy for now.
* find the next thread that can run and
* set the current thread ID to it,
* so that it can be restored (by the last line of
* this function).
* there should be at least one thread, so this loop
* will always terminate.
*/
    //flag = PSW;
    SAVESTATE;
    //currentThread = (currentThread + 1) % 2;
    if(currentThread == 0x01) currentThread = 0x00;
    else currentThread = 0x01;
    RESTORESTATE;
}

/*
* ThreadExit() is called by the thread's own code to termiate
* itself. It will never return; instead, it switches context
* to another thread.
*/
void ThreadExit(void){
/*
* clear the bit for the current thread from the
* bit map, 
*/
    EA = 0;
    if(currentThread == 0) bitmask = 0b1110;
    else if(currentThread == 1) bitmask = 0b1101;
    else if(currentThread == 2) bitmask = 0b1011;
    else bitmask = 0b0111;
    threadBitmap = (threadBitmap & bitmask);
    
    /* decrement thread count (if any),
    * and set current thread to another valid ID.
    * Q: What happens if there are no more valid threads?
    */
    threadCount--;
    SemaphoreSignal(semThreads);

    /* threadyield 1.0 */
    if(currentThread == 0){
        bitlook = threadBitmap >> 1;
        
        bitmask = 0b1110;
        if ((bitlook | bitmask ) != bitmask) currentThread = 0x01;
        else{
            bitmask = 0b1101;
            if ((bitlook | bitmask ) != bitmask) currentThread = 0x02;
            else{
                bitmask = 0b1011;
                if ((bitlook | bitmask ) != bitmask) currentThread = 0x03;
            }
        }   
    }
    else if(currentThread == 1){
        //determine bitlook
        bitlook = threadBitmap >> 2;
        
        bitmask = 0b0001;
        tid = (threadBitmap & bitmask) << 2;
        bitlook = bitlook | tid; 
        
        bitmask = 0b1110;
        if ((bitlook | bitmask ) != bitmask) currentThread = 0x02;
        else{
            bitmask = 0b1101;
            if ((bitlook | bitmask ) != bitmask) currentThread = 0x03;
            else{
                bitmask = 0b1011;
                if ((bitlook | bitmask ) != bitmask) currentThread = 0x00;
            }
        }
    }
    else if(currentThread == 2){
        bitlook = threadBitmap << 6; //14-8
        bitlook = threadBitmap >> 5;
        bitlook = bitlook | (threadBitmap >> 3);
        
        bitmask = 0b1110;
        if ((bitlook | bitmask ) != bitmask) currentThread = 0x03;
        else{
            bitmask = 0b1101;
            if ((bitlook | bitmask ) != bitmask) currentThread = 0x00;
            else{
                bitmask = 0b1011;
                if ((bitlook | bitmask ) != bitmask) currentThread = 0x01;
            }
        }
    }
    else{
        bitlook = threadBitmap << 5;
        bitlook = threadBitmap >> 5;
        
        bitmask = 0b1110;
        if ((bitlook | bitmask ) != bitmask) currentThread = 0x00;
        else{
            bitmask = 0b1101;
            if ((bitlook | bitmask ) != bitmask) currentThread = 0x01;
            else{
                bitmask = 0b1011;
                if ((bitlook | bitmask ) != bitmask) currentThread = 0x02;
            }
        }
    }
    
    // same wake up time support
    // after the 1st wake up and exit.
    // switch to 2nd 
    if(wakeQueueFlag == 1){
        currentThread = wakeQueue[1];
        wakeQueueFlag = 0;
        wakeQueue[1] = 0;
    }
    RESTORESTATE;
    EA = 1;
}
