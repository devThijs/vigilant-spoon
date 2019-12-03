
#ifndef lilfunctions_H_
#define lilfunctions_H_


#include <stdio.h>

#include <stdint.h>


#ifdef __cplusplus
	extern "C"{
#endif


#define ACC_ADDR 0b0011001
#define MAG_ADDR 0b0011110

void USART_putstring(char* StringPtr);
void USART_putheader(char* StringPtr, char Character);
void startRTC(uint16_t period);
int filterPAPin(int pinToMeasure, int resolution, double threshold);
int filterPBPin(int pinToMeasure, int resolution, double threshold);
void disableRTC();


#ifdef __cplusplus
}
#endif

#endif
