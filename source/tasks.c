#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include "tasks.h"
#include "pwm.h"

unsigned char check, input, cnt;
unsigned char blinkcnt;
unsigned char playbackFlag = 0;
unsigned char recordFlag = 0;
unsigned int i;
unsigned char hold1, hold2, hold3 = 0x00;
const double NoteArray[7] = {493.88, 440.00, 392.00, 349.23, 329.63, 293.66, 261.63};
uint8_t EEMEM NoteStorage[1000];
uint8_t ReadChar;

enum ParseInputStates{Parse, Sleep} ParseInputState;
enum LedOutputStates{GetOutput} LedOutputState;
enum PWM1State{Pulse} PWM1State;
enum RecordStates{Wait, Held, BlinkOn, BlinkOff, Record} RecordState;
enum PlaybackStates{WaitP, HeldP, Playback} PlaybackState;

char TurnBit (char Num) {
	if (Num == 7) {
		return 0x80;
		} else if (Num == 6) {
		return 0x40;
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
		} else if (Num == 7) {
		return 0x40;
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
		} else if (Bit == 0x40) {
		return 5;
		} else if (Bit == 0x80) {
		return 6;
		} else {
		return 0;
	}
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
		hold3 = 0x00;
		check = ~PINA & 0x7F;
		cnt = 1;
		while (cnt < 8 && hold3 == 0x00) {
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
			if (((check & 0x01) != 0) && hold3 == 0x00 && cnt < 8) {
				hold3 = TurnBit(cnt);
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
		input = hold1 | hold2 | hold3;
		PORTD = input;
		break;
	}
	return LedOutputState;
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
			set_PWM1(NoteArray[TurnNum(hold1)]);
		}
		break;
	}
	return PWM1State;
}

int RecordTick (int RecordState) {
	unsigned char recordCheck = ~PINB & 0x02;
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
		if (i < 1000) {
			RecordState = Record;
			i++;
			} else {
			RecordState = Wait;
			i = 0;
		}
		break;
	}
	switch (RecordState) {
		case Wait:
		recordFlag = 0;
		PORTC = 0x00;
		break;
		case Held:
		PORTC = 0x00;
		break;
		case BlinkOn:
		recordFlag = 1;
		PORTC = 0x01;
		break;
		case BlinkOff:
		recordFlag = 1;
		PORTC = 0x00;
		break;
		case Record:
		PORTC = 0x01;
		recordFlag = 1;
		ReadChar = (hold1 | hold2 | hold3);
		eeprom_write_byte(&NoteStorage[i], ReadChar);
		break;
	}
	return RecordState;
}

unsigned int PlaybackCnt;

int PlaybackTick (int PlaybackState) {
	unsigned char PlayCheck = ~PINB & 0x01;
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
		if (PlaybackCnt < 1000) {
			PlaybackState = Playback;
			PlaybackCnt++;
			} else {
			PlaybackState = Wait;
			PlaybackCnt = 0;
		}
		break;
	}
	switch (PlaybackState) {
		case WaitP:
		playbackFlag = 0;
		break;
		case HeldP:
		break;
		case Playback:
		PORTC = 0x02;
		playbackFlag = 1;
		hold1 = 0x00;
		hold2 = 0x00;
		hold3 = 0x00;
		ReadChar = eeprom_read_byte(&NoteStorage[PlaybackCnt]);
		check = (char)ReadChar;
		cnt = 1;
		while (cnt < 9 && hold3 == 0x00) {
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
			if (((check & 0x01) != 0) && hold3 == 0x00 && cnt < 9) {
				hold3 = TurnBitP(cnt);
				check = check >> 1;
				cnt++;
			}
			check = check >> 1;
			cnt++;
		}
		break;
	}
	return PlaybackState;
}

