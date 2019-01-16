/*
* file: testpreempt.c
*/
#include <8051.h>
#include "preemptive.h"
//#include "preemptive.c"
/*
* @@@ [2pt]
* declare your global variables here, for the shared buffer
* between the producer and consumer.
* Hint: you may want to manually designate the location for the
* variable. you can use
* _idata __at (0x30) type var;
* to declare a variable var of the type
*/
__idata __at (0x2A) char parkingLot; //semaphore for parking lot
__idata __at (0x2B) char syncspots; //mutex for parking spots
__idata __at (0x2C) char spots[2];

__idata __at (0x3B) char semThreads;

extern char time;
extern char n;
extern char currentThread;
extern char wakeQueueFlag;
extern char wakeQueue;
extern char sleep1;
extern char sleep2;
extern char sleep3;
//static __idata __at (0x35) char currentThread;
extern void delay(void);
/*
void Park(void){
    SemaphoreWait(parkingLot);
    // get the parking spots
    SemaphoreWait(syncspots);
    if(spots[0] == 0x04) spots[0] = currentThread;
    else spots[1] = currentThread;
    SemaphoreSignal(syncspots);
    // leave the car on the spots for some time
    n = now();
    delay();
    // exit the parking lot
    SemaphoreWait(syncspots);
    if(spots[0] == currentThread) spots[0] = 0x04;
    else spots[1] = 0x04;
    SemaphoreSignal(syncspots);
    SemaphoreSignal(parkingLot); 
    
    //???
}
*/
void Car(void){
    //try to go in the parkingLot
    SemaphoreWait(parkingLot);
    // get the right to park 
    SemaphoreWait(syncspots);
    if(spots[0] == 0) spots[0] = currentThread;
    else spots[1] = currentThread;
    SemaphoreSignal(syncspots);
    
    // leave the car on the spots for some time
    delay();
    if(currentThread == 1){while(sleep1 == 1);}
    else if(currentThread == 2){while(sleep2 == 1);}
    else{while(sleep3 == 1);}
    
    // exit the parking lot
    SemaphoreWait(syncspots);
    if(spots[0] == currentThread) spots[0] = 0;
    else spots[1] = 0;
    SemaphoreSignal(syncspots);
    SemaphoreSignal(parkingLot); 
    
    //while(1); 
    ThreadExit();
}


/* 
* main() is started by the thread bootstrapper as thread-0.
* It can create more thread(s) as needed:
* one thread can acts as producer and another as consumer.
*/
void main(void){
/*
* initialize globals
* set up Producer and Consumer.
* Because both are infinite loops, there is no loop
* in this function and no return.
*/
    SemaphoreCreate(semThreads, 3); 
    SemaphoreCreate(parkingLot, 2);
    SemaphoreCreate(syncspots, 1);
    spots[0] = 0x00; 
    spots[1] = 0x00;
    while(1){
        SemaphoreWait(semThreads); 
        ThreadCreate(Car);
    }
}

extern void Bootstrap(void);
void _sdcc_gsinit_startup( void ) {
    __asm 
        LJMP _Bootstrap
    __endasm;
}


void _mcs51_genRAMCLEAR( void ) {}
void _mcs51_genXINIT( void ) {}
void _mcs51_genXRAMCLEAR( void ) {}
void timer0_ISR( void ) __interrupt (1) {
    __asm
        ljmp _myTimer0Handler
    __endasm;
}