#ifndef _SPI_1284__
#define _SPI_1284__

#define RSPORT PORTB
#define rspin 0
#define RS 0			// RS/DC PIN
#define RES 1			// reset pin
void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	//PORTB = (1<<DDRB5)|(1<<DDRB7);
	DDRB = 0xFF;
	
	// initialization for 1284
	PORTB |= (1 << 0) | (1 << 1);
	PORTB |= (1 << 5) | (1 << 7);

	//initialization for 328p
	//PORTB |= (1 << 0) | (1 << 1);
	//PORTB |= (1 << 3) | (1 << 5);
	
	/* Enable SPI, Master, set clock rate fck/2 */
	SPCR = (1<<SPE)|(1<<MSTR)|(0<<SPR1)|(0 << SPR0);
	SPSR = (1 << SPI2X);
	_delay_ms(100);
}

void SoftwareReset(){
	RSPORT |= (1 << RES);
	_delay_ms(5);
	RSPORT &= ~(1 << RES);
	_delay_ms(20);
	RSPORT |= (1 << RES);
	_delay_ms(150);
}

inline void spiwrite(uint8_t c) {
	SPDR = c;
	while (!(SPSR & (1 << SPIF))) ;
}



void writedata(uint8_t c) {
	RSPORT |= (1 << RS);
	spiwrite(c);
}


void writecommand(uint8_t c) {
	RSPORT &= ~(1 << RS);
	spiwrite(c);
}


#endif