/*
 * uP_Module2.cpp
 *
 * Created: 3/4/2014 2:21:53 PM
 *  Author: Brandon
 * 
 * NOTES TO INCREASE PERFORMANCE:
 * the first half of triangle and sawtooth are the same, you can share those values
 * can store the LUTs in Progmem if wish
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "LUT.h"
#include "my_def.h"

int main(void)
{	
	ADC_init();
	SPI_init();
	IO_init();
	TC_init();
	
	//up the clock speed to 8MHZ
	CLKPR = 1<<CLKPCE;
	CLKPR &= 1<<CLKPCE; //switch to 8 MHZ 
	
	sei();
	
    while(1)
    {
       asm("nop");
    }
}

ISR(TIMER1_COMPA_vect)
{
	cli();
	
	prev_waveCode = waveCode;
	waveCode = getWaveCode();
	
	if((waveCode != prev_waveCode) || sample_count == 50)
	{
		sample_count = 0; //we change wave types so restart at 0 or we reached the end of our arrays
	}
	
	if(waveCode == 0) //draw sine
	{
		send_DAC_value(sin_LUT[sample_count++]);
	}
	else if(waveCode == 1) //draw square
	{
		if(sample_count < 25)
		{
			send_DAC_value(0); //half the period 0
		}
		else
		{
			send_DAC_value(1023); //other half the period VCC
		}
		sample_count++;
	}
	else if(waveCode == 2) //draw triangle
	{
		send_DAC_value(tri_LUT[sample_count++]);
	}
	else if(waveCode == 3) //draw sawtooth
	{
		send_DAC_value(saw_LUT[sample_count++]);
	}
	else //something wnet wrong, output 2.5 volts
	{
		send_DAC_value(1023/2);
	}
	
	//check the ADC and make the new OCR1AH and L accordingly
	
	if (ADCSRA & 1<<ADIF) //if a conversion is complete, update the new top
	{
		current_ADC = (uint32_t) getADCVal();
		current_ADC *= (uint32_t) (16000 - 1600); //top is equal to ADC(2000-200)/1023 + 200
		current_ADC /= (uint32_t) 1023;
		current_ADC += (uint32_t) 1600;
		
		OCR1AH = (0xFF00 & current_ADC) >> 8;
		OCR1AL = 0xFF & current_ADC; //new top set for next round
	}
	
	sei();
}

void inline ADC_init()
{	
	ADMUX |= (1<<REFS0); //set AVCC as reference (5V)
	ADCSRA |= 1<<ADEN | 1<<ADSC | 1<<ADATE | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0; //enable ADC, enable interrupt, start free running mode, divide clock by max(save power)
}

void inline SPI_init()
{
	DDRB |= (1<<MOSI_BIT | 1<<SCK_BIT | 1<<PORTB4); // /ss, sck, and mosi all outputs
	SPSR0 |= 1<<SPI2X0;
	SPCR0 |= (1<<SPE0 | 1<<MSTR0); //enable in master mode, fosc/4 frequency ie 2 mhz
	PORTB |= 1<<PORTB4; //make /ss high
}

void inline IO_init() //the two LSB of IO port are inputs, determine what kind of wave
{	
	IOPORT_DIR &= (~0b11); //make bits 1 and 0 inputs
}

void inline TC_init()
{
	//if I don't prescale the clock, the lowest number I'll count to is 200, since I'll
	//be changing at a frequency of 100Hz * 50 samples = 5000 Hz
	//so I need to reach top of counter at a frequency of 5000 Hz
	//1e6/5000 is 200 which is the least I'll count up to
	//the most is 1e6/500 (500 is 50 * 10 hz) which is 2000
	
	TIMSK1 |= (1<<OCIE1A); //allow interrupts when I reach OCA
	OCR1AH = (0xFF00 & 16000) >> 8;
	OCR1AL = (0xFF & 16000);  //set top to 16000, default frequency of 10 hz	
	TCCR1B |= (1<<WGM12 | 1<< CS10); //turn on CTC mode, do not prescale the clock
}

uint8_t getWaveCode()
{
	uint8_t retval = IOPORT_IN;
	retval &= 0b11;
	return retval;
}

uint16_t getADCVal()
{
	uint16_t ADC_Low = 0;
	uint16_t ADC_High = 0;
	uint16_t retval = 0;
	
	ADC_Low = ADCL;
	ADC_High = ADCH << 8; //shift by 8 bits for the sum
	
	retval = ADC_Low + ADC_High;
	
	ADCSRA |= 1<<ADIF; //clear the conversion done bit
	
	return retval;
}

void send_DAC_value(uint16_t val)
{
	PORTB &= ~(1<<PORTB4); //turn off /ss
	
	//code to update outputs and load to DACA is 1001
	val = val << 2;
	val |= (0b1001 << 12);
	
	SPDR0 = val >> 8; //upper byte first
	
	while(!(SPSR0 & 1<<SPIF0)); //wait until SPIF is set
	
	SPDR0 = (0xFF & val); //send lower byte
	
	while(!(SPSR0 & 1<<SPIF0)); //wait until lower byte is done
	
	PORTB |= 1<<PORTB4; //toggle /ss to finish
}