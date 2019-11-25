/*	Author: akim106
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Custom Project
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "scheduler.h"
#include "timer.h"
#include "tasks.h"
#include "tasks.c"
#endif

int main(void) {
    unsigned long GCD = 10;
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;

    static task Input, LedOutput;
    task *tasks[] = {&Input, &LedOutput};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    Input.state = Combine;
    Input.period = 10;
    Input.elapsedTime = Input.period;
    Input.TickFct = &InputTick;

    LedOutput.state = Light;
    LedOutput.period = 10;
    LedOutput.elapsedTime = LedOutput.period;
    LedOutput.TickFct = &LedOutputTick;

    TimerSet(GCD);
    TimerOn();
   
    
    while (1) {
      for (int i=0; i < numTasks; i++) {
        if (tasks[i]->elapsedTime == tasks[i]->period) {
          tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
          tasks[i]->elapsedTime = 0;   
        }
        tasks[i]->elapsedTime += GCD;
      }
      while(!TimerFlag);
      TimerFlag = 0;
    }
    return 1;
}
