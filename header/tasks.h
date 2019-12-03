#ifndef __tasks_h__
#define __tasks_h__

int ParseInputTick (int ParseInputState);
int LedOutputTick (int LedOutputState);
int ADCTick (int ADCInputState);
int PWM1Tick (int PWM1State);
int PWM2Tick (int PWM2State);
int RecordTick (int RecordState);
int PlaybackTick (int PlaybackState);

#endif

