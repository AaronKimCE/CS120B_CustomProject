#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "pwm.h"

void set_PWM1 (double frequency) {
	static double current_frequency1;
	if (frequency != current_frequency1) {
		if (!frequency) {TCCR3B &= 0x08;}
		else {TCCR3B |= 0x03;}
		if (frequency < 0.954) {OCR3A = 0xFFFF;}
		else if (frequency > 31250) {OCR3A = 0x0000;}
		else {OCR3A = (short) (8000000 / (128 * frequency)) - 1;}

		TCNT3 = 0;
		current_frequency1 = frequency;
	}
}

void PWM1_on () {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM1(0);
}

void PWM1_off () {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

