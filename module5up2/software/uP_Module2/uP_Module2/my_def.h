//Brandon Pollack
//Definitions and Declarations
#define F_CPU 8e6UL
#define IOPORT PORTC //Port C is the IO port
#define IOPORT_DIR DDRC
#define IOPORT_IN PINC;

void inline ADC_init();
void inline SPI_init();
void inline IO_init();
void inline TC_init();

uint8_t getWaveCode(); //returns 0-3
uint16_t getADCVal(); //get the value in the result register

void send_DAC_value(uint16_t val); //send a 10 bit DAC value, with the 4 bit write to A command appended

volatile uint8_t waveCode = 0, prev_waveCode = 0; //4 different values, 0 sine, 1 square, 2 triangle, 3 sawtooth

volatile uint8_t sample_count = 0; //variable to keep track of what value we are writing

volatile uint32_t current_ADC = 0;
