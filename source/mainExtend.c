/*
 * mainExtend.c
 * 
 * Created: 12/2/2019 7:07:53 PM
 * Author : Aaron Kim
 */ 

#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "scheduler.h"
#include "timer.h"
#include "pwm.c"
#include "usart.h"

unsigned char data = 0;

enum PWMTickStates{Parse} PWMTickState;
const double NoteArray[7] = {493.88, 440.00, 392.00, 349.23, 329.63, 293.66, 261.63};

int TurnNum (char Bit) {
	if (Bit == 0x01) {
		return 0;
		} else if (Bit == 0x02) {
		return 1;
		} else if (Bit == 0x04) {
		return 2;
		} else if (Bit == 0x08) {
		return 3;
		} else if (Bit == 0x10) {
		return 4;
		} else if (Bit == 0x20) {
		return 5;
		} else if (Bit == 0x80) {
		return 6;
		} else {
		return 0;
	}
}

int PWM2Tick (int PWMTickState) {
	switch (PWMTickState) {
		case Parse:
		PWMTickState = Parse;
		break;
	}
	switch (PWMTickState) {
		case Parse:
		if (USART_HasReceived()) {
		    data = USART_Receive();
			USART_Flush();
		}
		if (data == 0x00) {
			set_PWM1(0);
		} else {
			set_PWM1(NoteArray[TurnNum(data)]);
		}
		break;
	}
	return PWMTickState;
}

int main(void)
{
	unsigned long GCD = 5;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFE; PORTD = 0x00;
	
	static task PWM2;
	task *tasks[] = {&PWM2};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    PWM2.state = Parse;
    PWM2.period = 5;
    PWM2.elapsedTime = PWM2.period;
    PWM2.TickFct = &PWM2Tick;
	
	TimerSet(GCD);
	TimerOn();
	PWM1_on();
	set_PWM1(0);
	initUSART();
	
    while (1) 
    {
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


