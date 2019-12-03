#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include "tasks.h"
#include "pwm.h"
#include "usart.h"

unsigned char check, input, cnt;
unsigned char blinkcnt;
unsigned char playbackFlag = 0;
unsigned char recordFlag = 0;
unsigned int i;
unsigned char hold1, hold2 = 0x00;
unsigned char adcv = 0;
const double NoteArray[7] = {493.88, 440.00, 392.00, 349.23, 329.63, 293.66, 261.63};
unsigned int duration = 1;
uint8_t EEMEM NoteStorage[10000];
uint8_t ReadChar;

enum ParseInputStates{Parse, Sleep} ParseInputState;
enum LedOutputStates{GetOutput} LedOutputState;
enum ADCInputStates{GetADC, SleepADC} ADCInputState;
enum PWM1State{Pulse} PWM1State;
enum PWM2State{Pulse2} PWM2State;
enum RecordStates{Wait, Held, BlinkOn, BlinkOff, Record, StopR} RecordState;
enum PlaybackStates{WaitP, HeldP, Playback, StopP} PlaybackState;

char TurnBit (char Num) {
	if (Num == 7) {
		return 0x80;
		} else if (Num == 6) {
		return 0x20;
		} else if (Num == 5) {
		return 0x10;
		} else if (Num == 4) {
		return 0x08;
		} else if (Num == 3) {
		return 0x04;
		} else if (Num == 2) {
		return 0x02;
		} else if (Num == 1) {
		return 0x01;
		} else {
		return 0x00;
	}
}

char TurnBitP (char Num) {
	if (Num == 8) {
		return 0x80;
		} else if (Num == 6) {
		return 0x20;
		} else if (Num == 5) {
		return 0x10;
		} else if (Num == 4) {
		return 0x08;
		} else if (Num == 3) {
		return 0x04;
		} else if (Num == 2) {
		return 0x02;
		} else if (Num == 1) {
		return 0x01;
		} else {
		return 0x00;
	}
}

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

double ConvertModifier (char adcinput) {
	if (adcinput >= 105 && adcinput < 165) {
		return 0;
	} else if (adcinput >= 85 && adcinput < 105) {
		return 4;
	} else if (adcinput >= 65 && adcinput < 85) {
		return 9;
	} else if (adcinput >= 45 && adcinput < 65) {
		return 14;
	} else if (adcinput >= 15 && adcinput < 45) {
		return 17;
	} else if (adcinput >= 0 && adcinput < 15) {
		return 21;
	} else if (adcinput >= 165 && adcinput < 185) {
		return -4;
	} else if (adcinput >= 185 && adcinput < 215) {
		return -9;
	} else if (adcinput >= 215 && adcinput < 235) {
		return -14;
	} else if (adcinput >= 235 && adcinput < 245) {
		return -17;
	} else if (adcinput >= 245) {
		return -21;
	} else {
		return 0;
	}
}

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	ADMUX |= (1 << ADLAR) | (1 << MUX1);
}

int ParseInputTick (int ParseInputState) {
	switch (ParseInputState) {
		case Parse:
		if (playbackFlag == 0) {
			ParseInputState = Parse;
			} else {
			ParseInputState = Sleep;
		}
		break;
		case Sleep:
		if (playbackFlag == 0) {
			ParseInputState = Parse;
			} else {
			ParseInputState = Sleep;
		}
		break;
	}
	switch (ParseInputState) {
		case Parse:
		hold1 = 0x00;
		hold2 = 0x00;
		check = ~PINC & 0x7F;
		cnt = 1;
		while (cnt < 8 && hold2 == 0x00) {
			if (((check & 0x01) != 0) && hold1 == 0x00 && cnt < 8) {
				hold1 = TurnBit(cnt);
				check = check >> 1;
				cnt++;
			}
			if (((check & 0x01) != 0) && hold2 == 0x00 && cnt < 8) {
				hold2 = TurnBit(cnt);
				check = check >> 1;
				cnt++;
			}
			check = check >> 1;
			cnt++;
		}
		break;
		case Sleep:
		break;
	}
	return ParseInputState;
}

int LedOutputTick (int LedOutputState) {
	switch (LedOutputState) {
		case GetOutput:
		LedOutputState = GetOutput;
		break;
	}
	switch (LedOutputState) {
		case GetOutput:
		input = hold1 | hold2;
		PORTB = input;
		break;
	}
	return LedOutputState;
}

int ADCTick (int ADCInputState) {
	switch (ADCInputState) {
		case GetADC:
		if (recordFlag == 0) {
			ADCInputState = GetADC;
		} else {
			ADCInputState = SleepADC;
		}
		break;
		case SleepADC:
		if (recordFlag == 0) {
			ADCInputState = GetADC;
		} else {
			ADCInputState = SleepADC;
		}
		break;
	}
	switch (ADCInputState) {
		case GetADC:
		adcv = ADCH;
		break;
		case SleepADC:
		break;
	}
	return ADCInputState;
}

int PWM1Tick (int PWM1State) {
	switch (PWM1State) {
		case Pulse:
		PWM1State = Pulse;
		break;
	}
	switch (PWM1State) {
		case Pulse:
		if (hold1 == 0x00) {
			set_PWM1(0);
			} else {
			set_PWM1(NoteArray[TurnNum(hold1)] + ConvertModifier(adcv));
		}
		break;
	}
	return PWM1State;
}

int PWM2Tick (int PWM2State) {
	switch (PWM2State) {
		case Pulse2:
		PWM2State = Pulse2;
		break;
	}
	switch (PWM2State) {
		case Pulse2:
		if (hold2 == 0x00) {
			if (USART_IsSendReady()) {
			  USART_Send(0x00);
			  USART_Flush();
			} 
		} else {
			if (USART_IsSendReady()) {
			  USART_Send(hold2);
			  USART_Flush();
			}
		}
		break;
	}
	return PWM2State;
}

int RecordTick (int RecordState) {
	unsigned char recordCheck = ~PINA & 0x02;
	switch (RecordState) {
		case Wait:
		if (recordCheck && (playbackFlag == 0)) {
			RecordState = Held;
			} else {
			RecordState = Wait;
		}
		break;
		case Held:
		if (!recordCheck) {
			RecordState = BlinkOn;
			i = 0;
			blinkcnt = 0;
			} else if (recordCheck) {
			RecordState = Held;
		}
		break;
		case BlinkOn:
		if (blinkcnt == 3) {
			RecordState = Record;
			i = 0;
			} else if (i < 50) {
			RecordState = BlinkOn;
			i++;
			} else {
			RecordState = BlinkOff;
			i = 0;
		}
		break;
		case BlinkOff:
		if (i < 50) {
			RecordState = BlinkOff;
			i++;
			} else {
			RecordState = BlinkOn;
			i = 0;
			blinkcnt++;
		}
		break;
		case Record:
		if (i < 10000 && !recordCheck) {
			RecordState = Record;
			i++;
		} else if (recordCheck) {
			RecordState = StopR;
			duration = i;
			i = 0;
		} else {
			RecordState = Wait;
			duration = i;
			i = 0;
		}
		break;
		case StopR:
		if (recordCheck) {
			RecordState = StopR;
		} else {
			RecordState = Wait;
		}
	}
	switch (RecordState) {
		case Wait:
		recordFlag = 0;
		PORTD = 0x00;
		break;
		case Held:
		recordFlag = 1;
		PORTD = 0x00;
		break;
		case BlinkOn:
		recordFlag = 1;
		PORTD = 0x40;
		break;
		case BlinkOff:
		recordFlag = 1;
		PORTD = 0x00;
		break;
		case Record:
		PORTD = 0x40;
		recordFlag = 1;
		ReadChar = (hold1 | hold2);
		eeprom_write_byte(&NoteStorage[i], ReadChar);
		break;
		case StopR:
		recordFlag = 1;
		break;
	}
	return RecordState;
}

unsigned int PlaybackCnt;

int PlaybackTick (int PlaybackState) {
	unsigned char PlayCheck = ~PINA & 0x01;
	switch (PlaybackState) {
		case WaitP:
		if (PlayCheck && (recordFlag == 0)) {
			PlaybackState = HeldP;
			} else {
			PlaybackState = WaitP;
		}
		break;
		case HeldP:
		if (PlayCheck) {
			PlaybackState = HeldP;
			} else {
			PlaybackState = Playback;
			PlaybackCnt = 0;
		}
		break;
		case Playback:
		if (PlayCheck && PlaybackCnt > 100) {
			PlaybackState = StopP;
			PlaybackCnt = 0;
		} else if (PlaybackCnt < duration) {
			PlaybackState = Playback;
			PlaybackCnt++;
		} else {
			PlaybackState = Wait;
			PlaybackCnt = 0;
		}
		break;
		case StopP:
		if (PlayCheck) {
			PlaybackState = StopP;
		} else {
			PlaybackState = Wait;
		}
		break;
	}
	switch (PlaybackState) {
		case WaitP:
		playbackFlag = 0;
		break;
		case HeldP:
		playbackFlag = 1;
		break;
		case Playback:
		PORTD = 0x20;
		playbackFlag = 1;
		hold1 = 0x00;
		hold2 = 0x00;
		ReadChar = eeprom_read_byte(&NoteStorage[PlaybackCnt]);
		check = (char)ReadChar;
		cnt = 1;
		while (cnt < 9 && hold2 == 0x00) {
			if (((check & 0x01) != 0) && hold1 == 0x00 && cnt < 9) {
				hold1 = TurnBitP(cnt);
				check = check >> 1;
				cnt++;
			}
			if (((check & 0x01) != 0) && hold2 == 0x00 && cnt < 9) {
				hold2 = TurnBitP(cnt);
				check = check >> 1;
				cnt++;
			}
			check = check >> 1;
			cnt++;
		}
		break;
		case StopP:
		playbackFlag = 1;
		break;
	}
	return PlaybackState;
}

