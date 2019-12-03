#include <atmel_start.h>
//#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include <usart_basic.h>

#include <stdbool.h>
#include <stdlib.h>
#include <i2c_simple_master.h>
#include <stdio.h>
#include <stdint.h>
#include <compiler.h>
#include <float.h>
#include <util/atomic.h>

bool isbitclear(uint8_t registeradress, uint8_t bit){   //insert register and bit to check (PER.reg, bit)
  if  (registeradress & (1<<bit)){
    return true;
  }
  else  { return false;}
}

// write16bitReg(uint8_t registeradress, uint16_t value){
//   uint8_t lowbyte;
//   uint8_t highbyte;

//   lowbyte = ( value & (0b11111111) );
//   highbyte = ( value & (255<<8) )>>8;

//   registeradress = lowbyte;
//   registeradress + 0x01 = highbyte;
// }

// clearbit(uint8_t registeradress, uint16_t value)

//----------------------------------------Real Time Counter---------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////////

//enable Real Time Counter in ATtiny3216 / ATtiny1616
//an interrupt activated after a compare match condition, this happens one count after counter equals compare register value.
//32khz and 32 divider: 1second resolution?
//1 configure desired oscillator in CLKCTRL, and write CLKSEL bits
//2 set compare value (RTC.CMP)
//3 enable the desired interrupts by writing to the respective Interrupt Enable bits (CMP, OVF) in theInterrupt Control register (RTC.INTCTRL).
//4 configure the RTC internal prescaler and enable the RTC by writing the desired value to thePRESCALER bit field and a '1' to the RTC Enable bit (RTCEN) in the Control A register(RTC.CTRLA).
// The RTC peripheral is used internally during device start-up. Always check the Busy bits in the RTC.STATUS and RTC.PITSTATUS registers, also on initial configuration.
//interrupt vector : <RTC>


void USART_putstring(char* StringPtr){
		USART_0_write('>');
	while(*StringPtr != 0x00){
		USART_0_write(*StringPtr);
	StringPtr++;}

}

#define linelength 30
//a nice formatted title header for debugging
void USART_putheader(char* StringPtr, char Character){

	int stringsize = 0;
	int linesize = 0;
	int counter = 0;
	bool oddNumber = false;
	while(*StringPtr != 0x00){
		stringsize++;
	StringPtr++;}
	StringPtr = StringPtr - stringsize;


	if ((linelength-stringsize)%2){
		oddNumber = true;
	}

	
	linesize = (linelength - stringsize)/2;
	
	while(counter<=linesize){
		USART_0_write(Character);
	counter++;}
	counter = 0;


	while(*StringPtr != 0x00){
		USART_0_write(*StringPtr);
	StringPtr++;}

	while(counter<=linesize){
		USART_0_write(Character);
	counter++;}
	
	if (oddNumber){
		USART_0_write(Character);
	}
	USART_0_write(0xA);
}

union bitsplit{
	struct {
		uint8_t	bit8Low;
		uint8_t	bit8High;
	};
	uint16_t bit16;
}bit_split;

void startRTC(uint16_t period){			//(clock ticks until interrupt)

	while (RTC.STATUS>=1){}				//wait if ctrlabusy flag set
	RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;	//(0x0, INT32K32.768 kHz from OSCULP32K)
	bit_split.bit16 = period;


	RTC.PERL	=	bit_split.bit8Low;
	RTC.PERH	=	bit_split.bit8High;			//set period

	RTC.INTCTRL = 1;					//overflow interrupt enable

	while (RTC.STATUS>=1){}				//wait if ctrlabusy flag set
/*	RTC.CTRLA |= (0b00101000);			//set 32k div prescaler, 1.024ms scale*/
	RTC.CTRLA |= 1	;					//enable  real time counter
}

void disableRTC(){
	while (RTC.STATUS>=1){}				//wait if ctrlabusy flag set
	RTC.CTRLA &= ~1	;
}
//don't forget to clear interrupt flag in ISR
//------------------------------------------------------------------------------------------------------


//-----------------------------------------------Pins --------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////////
int filterPAPin(int pinToMeasure, int resolution, double threshold){//read pin and filter input of pin, returns high or low. pinToMeasure 0-7, resolution is # of datapoints, time in ms threshold in 0-1


	double mVar=0;
	for (int x=0; x<resolution; x++){
		if (PORTA.IN & (1<<(pinToMeasure))	){
			mVar++;		}
		_delay_ms(10);
	}

	if (	(double)mVar/resolution > threshold){
		return 1;
	}
	else if(	(double)mVar/resolution < (1 - threshold)	){

		return 0;}
	else {
		return 2;
	}
}

int filterPBPin(int pinToMeasure, int resolution, double threshold){//read pin and filter input of pin, returns high or low. pinToMeasure 0-7, resolution is # of datapoints, time in ms threshold in 0-1


	double mVar=0;
	for (int x=0; x<resolution; x++){
		if (PORTB.IN & (1<<(pinToMeasure))	){
		mVar++;		}
		_delay_ms(10);
	}

	if (	(double)mVar/resolution > threshold){
		return 1;
	}
	else if(	(double)mVar/resolution < (1 - threshold)	){

	return 0;}
	else {
		return 2;
	}
}



//-----------------------------------------------timer--------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////CLK = Mainclock / 1024//////////////////////////////////////////
// int8_t TIMERinit(uint8_t compare0, uint8_t compare1, uint8_t compare2)		//compare - amount of clock ticks to compare to - write 0 to disable corresponding compare
// {
//
// 	TCA0.SINGLE.CMP0 = compare0; /* Compare Register 0: 0x10 */
// 	if (compare1>0){
// 		TCA0.SINGLE.CMP1 = compare1; /* Compare Register 1: 0x0 */
// 	}
// 	if (compare2>0){
// 		TCA0.SINGLE.CMP2 = compare2; /* Compare Register 2: 0x0 */
// 	}
//
// 	// TCA0.SINGLE.CNT = 0x0; /* Count: 0x0 */
//
// 	TCA0.SINGLE.CTRLB = 0 << TCA_SINGLE_ALUPD_bp       /* Auto Lock Update: disabled */
// 	| 1 << TCA_SINGLE_CMP0EN_bp    /* Compare 0 Enable: enabled */
// 	| (compare1>0?1:0) << TCA_SINGLE_CMP1EN_bp    /* Compare 1 Enable: disabled */
// 	| (compare2>0?1:0) << TCA_SINGLE_CMP2EN_bp    /* Compare 2 Enable: disabled */
// 	| TCA_SINGLE_WGMODE_NORMAL_gc; /*  */
//
// 	// TCA0.SINGLE.CTRLC = 0 << TCA_SINGLE_CMP0OV_bp /* Compare 0 Waveform Output Value: disabled */
// 	//		 | 0 << TCA_SINGLE_CMP1OV_bp /* Compare 1 Waveform Output Value: disabled */
// 	//		 | 0 << TCA_SINGLE_CMP2OV_bp; /* Compare 2 Waveform Output Value: disabled */
//
// 	// TCA0.SINGLE.DBGCTRL = 0 << TCA_SINGLE_DBGRUN_bp; /* Debug Run: disabled */
//
// 	// TCA0.SINGLE.EVCTRL = 0 << TCA_SINGLE_CNTEI_bp /* Count on Event Input: disabled */
// 	//		 | TCA_SINGLE_EVACT_POSEDGE_gc; /* Count on positive edge event */
//
// 	TCA0.SINGLE.INTCTRL = 1 << TCA_SINGLE_CMP0_bp   /* Compare 0 Interrupt: enabled */
// 	| (compare1>0?1:0) << TCA_SINGLE_CMP1_bp /* Compare 1 Interrupt: disabled */
// 	| (compare2>0?1:0) << TCA_SINGLE_CMP2_bp /* Compare 2 Interrupt: disabled */
// 	| 0 << TCA_SINGLE_OVF_bp; /* Overflow Interrupt: disabled */
//
// 	// TCA0.SINGLE.PER = 0xffff; /* Period: 0xffff */
//
// 	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc /* System Clock / 1024 */
// 	| 1 << TCA_SINGLE_ENABLE_bp; /* Module Enable: enabled */
//
// 	return 0;
// }
