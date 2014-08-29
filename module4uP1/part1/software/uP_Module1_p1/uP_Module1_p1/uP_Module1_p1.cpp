/*
 * uP_Module1_p1.cpp
 *
 * Created: 2/10/2014 5:25:15 PM
 *  Author: Brandon
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#define OCVAL 250

//functions
void inline io_init();
void inline tc_init();

//variables used in interrupts
uint8_t input = 0;

//remember clock is 1 MHZ
int main()
{	
	io_init();
	tc_init();
		
    while(1)
    {
		input = PINA & (1); //input is the LSB of porta
		if(input == 1)
		{
			//change div in TCCR1B to be 2k Hz(div1), turn off OCB, turn on OCA
			DDRD &= (0xFF & ~(1<<DDD4)); //clear DDR4 OCB
			DDRD |= 1<<DDD5; //set DDR5 OCA
			TCCR1B &= (1<<WGM12 | 1<<CS10);
		}
		else
		{
			//change div in TCCR1B to be 2 Hz (div1024), turn off OCA, on OCB
			DDRD &= (0xFF & ~(1<<DDD5)); //clear DDR5 OCA
			DDRD |= 1<<DDD4; //set DDR4 OCB
			TCCR1B |= 1<<CS12;
		}
    }
}

void inline io_init()
{
	DDRA = 0; //make sure that port A is set to input	
	//make direction of OCA 1 (not sure which pin it is yet)
}

void inline tc_init()
{
	TCCR1A |= 1<<COM1A0 | 1<<COM1B0; //turn on output compare A
	OCR1AH = (0xFF00 & OCVAL)>>8;
	OCR1AL = OCVAL;
	OCR1BH = (0xFF00 & OCVAL)>>8;
	OCR1BL = OCVAL;	
	TCCR1B |= 1<<WGM12 | 1<<CS10; //WG is CTC mode (OCA is now top) and cs is 1 for sound	
	//last statement also starts TC
}