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
#include <avr/eeprom.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "scheduler.h"
#include "timer.h"
#include "tasks.h"
#include "tasks.c"
#include "pwm.h"

int main(void) {
    unsigned long GCD = 10;
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFC; PORTB = 0x03;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    static task ParseInput, LedOutput, PWM1, Record, Playback;
    task *tasks[] = {&ParseInput, &LedOutput, &PWM1, &Record, &Playback};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    ParseInput.state = Parse;
    ParseInput.period = 10;
    ParseInput.elapsedTime = ParseInput.period;
    ParseInput.TickFct = &ParseInputTick;

    LedOutput.state = GetOutput;
    LedOutput.period = 10;
    LedOutput.elapsedTime = LedOutput.period;
    LedOutput.TickFct = &LedOutputTick;

    PWM1.state = Pulse;
    PWM1.period = 10;
    PWM1.elapsedTime = PWM1.period;
    PWM1.TickFct = &PWM1Tick;
	
    Record.state = Wait;
    Record.period = 10;
    Record.elapsedTime = Record.period;
    Record.TickFct = &RecordTick;

    Playback.state = WaitP;
    Playback.period = 10;
    Playback.elapsedTime = Playback.period;
    Playback.TickFct = &PlaybackTick;

    TimerSet(GCD);
    TimerOn();
    PWM1_on();
    set_PWM1(0);
    
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

