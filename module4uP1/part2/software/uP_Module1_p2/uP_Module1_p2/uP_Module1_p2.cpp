/*
 * uP_Module1_p2.cpp
 *
 * Created: 2/12/2014 1:59:32 PM
 *  Author: Brandon
 */ 

//the ADC does not seem to be in free running mode for some reason
//it also doesn't seem to get another value after the first even when you startr another conv manually
#define F_CPU 1e6

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#define ERROR 3
#define R_KNOWN 51e3
#define MAXRES 1e6 + 1e5/2
#define MINRES 1e3 - 1e2/2
#define LCDPORT PORTC
#define LCDPORT_DIR DDRC

void inline lcd_init();
void inline adc_init();

void print_char_lcd(char& a);//print a single char
void printLCD(char* a); //print character string terminating in null
void lcd_bf();
void lcd_cmd(uint8_t CMD);

volatile uint32_t ADC_val = 0, prev_ADC_val = 0; //value obtained by ADC interrupt
const char out_of_range[] = "OUT OF RANGE";

volatile bool new_val_ready;

int main(void)
{
	adc_init();
	lcd_init();
	
	static char a[10];
	static uint32_t resistance = 0;
	
	new_val_ready = false;

	sei();
	
    while(1)
    {
		if(new_val_ready) //returns true if there was a new value
		{
			new_val_ready = false;
			
			if(ADC_val == 0)
			{
				resistance = 0;
			}
			else
			{
				resistance = R_KNOWN;
				resistance *= 1024;
				resistance /= ADC_val;
				resistance -= R_KNOWN; //formula for resistance
			}
			
			lcd_cmd(0x01); //clear screen before writing
			
			if(resistance > MAXRES || resistance < MINRES)
			{
				sprintf(a," %lu",ADC_val);
				printLCD((char*)out_of_range);
				printLCD(a);
				for(int i = 0; i < 20; i++)
				{
					a[i] = 0;
				}
			}
			else
			{
				sprintf(a, "%lu %lu", resistance, ADC_val);
				printLCD(a);
				for(int i = 0; i < 20; i++)
				{
					a[i] = 0; //zero out the array so the next value doesn't  have a printing issue if it has less digits
				}
			}
			//ADCSRA |= 1<<ADSC;
		}
    }
}

ISR(ADC_vect)
{
	cli(); //turn off interrupts
	
	uint16_t ADC_Low = 0;
	uint16_t ADC_High = 0;
	
	ADC_Low = ADCL;
	ADC_High = ADCH << 8; //shift by 8 bits for the sum
	
	prev_ADC_val = ADC_val;
	ADC_val = ADC_High + ADC_Low;
	
	if(prev_ADC_val + ERROR >= ADC_val && prev_ADC_val - ERROR <= ADC_val) //if I am within a margin of error, this is just ripple
	{
		ADC_val = prev_ADC_val; //so don't change ADC value and don't reprint
	}
	else
	{
		new_val_ready = true; //otherwise leave the change to ADC_val and reprint
	}
	
	sei(); //turn them back on
}

void print_char_lcd(char& data)
{
	LCDPORT_DIR = 0xFF;
	char upper = (data & 0xF0) >> 4 | 0xA0; //upper data
	char lower = (data & 0x0F) | 0xA0; //lower data
	
	//notice the A instead of the 8, this makes RS true, selecting Data Reg
	
	LCDPORT = upper; //send cmd upper
	_delay_ms(5);
	LCDPORT ^= (0x80); //toggle enable
	_delay_ms(5);
	
	LCDPORT = lower;
	_delay_ms(5);
	LCDPORT ^= (0x80); //same thing but for lower
	_delay_ms(5);
}

void printLCD(char* a)
{
	for(int i = 0; a[i] != 0; i++)
	{
		print_char_lcd(a[i]);
	}
}

void inline lcd_init()
{
	//LCD port format
	//<Enable><NOTHING><RS><RW><DB7:4> and DB 3 : 0 are not connected
	
	_delay_ms(100); //need to delay at least 40ms
	
	LCDPORT_DIR = 0xFF; //using LSBits of porta
	//see manual for init details
	
	lcd_cmd(0x33);
	lcd_cmd(0x32);
	lcd_cmd(0x2C);
	lcd_cmd(0x0C);
	lcd_cmd(0x01);
}

void lcd_cmd(uint8_t CMD)
{
	LCDPORT_DIR = 0xFF;
	uint8_t upper = ((CMD & 0xF0) >> 4); //upper command
	uint8_t lower = (CMD & 0x0F); //lower command
	
	LCDPORT = upper | 1<<7; //send cmd upper
	_delay_ms(10);
	LCDPORT ^= (0x80); //toggle enable
	_delay_ms(10);
	
	LCDPORT = lower | 1<<7;
	_delay_ms(10);
	LCDPORT ^= (0x80); //same thing but for lower
	_delay_ms(10);	
}

void inline adc_init()
{
	ADMUX |= (1<<REFS0); //set AVCC as reference (5V)
	ADCSRA |= 1<<ADEN | 1<<ADIE | 1<<ADSC | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0; //enable ADC, enable interrupt, start free running mode, divide clock by 128
}