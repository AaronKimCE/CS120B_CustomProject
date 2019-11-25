#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "tasks.h"

unsigned char check, input, cnt, allot;

enum Input{Combine};
enum LedOutput{Light};

char TurnBit (char Num) {
  if (Num == 7) {
    return 0x40;
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

int InputTick (int InputState) {
  switch (InputState) {
    case Combine:
      InputState = Combine;
      break;
  }
  switch (InputState) {
    case Combine:
      input = 0x00;
      check = ~PINA & 0x7F;
      cnt = 1;
      allot = 3;
      while (cnt < 8) {
        if ((check & TurnBit(cnt)) && allot != 0) {
          input = input | 0x01;
          allot--;
        }
        cnt++;
        input = input << 1;
      }      
      break;
  }
  return InputState;
}

int LedOutputTick (int LedOutputState) {
  switch (LedOutputState) {
    case Light:
      PORTB = input; 
      break;
  }
  switch (LedOutputState) {
    case Light:
      LedOutputState = Light;
      break;
  }
  return LedOutputState;
}
