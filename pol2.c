#define F_CPU 16000000UL    // Setting Frecquency
#include <avr/io.h> 
#include <util/delay.h>
#include <avr/interrupt.h>
#define direction DDRB       
#define databus PORTD
#define control PORTB
#define rs 0                // setting register select to pin 0
#define rw 1				// Settingng sread write to pin 1
#define en 2				// setting enable to pin 2 
int iter1 = 0;
int iter2 = 0;
void datawrite(char data)
{
	databus = (data & 0xF0); // creating a nibble
	control |= 1<<rs;        // setting register select to 1, i.e. Data Write mode
	control &=~(1<<rw);		// setting read write pin to 0, i.e setting up for read mode
	control |= 1<<en;		// Setting enable line to 1
	_delay_us(200);
	 control &= ~(1<<en);	// setting enable to 0
	databus = ((data <<4) & 0xF0);	
	control |= 1<<rs;	// setting register select to 1 
	control &=~(1 << rw); // setting read write pin to 0, i.e setting up for read mode
	control |= 1<<en; // Setting enable line to 1 
	_delay_us(200);
	 control &= ~(1<<en);   // again setting enable to 0
	_delay_us(200);
}
void printstring(int val)
{
	int i;
	char s[10];
	itoa(val,s,10);
	for(i = 0; s[i] != '\0'; i++)
	{
		datawrite(s[i]);
		_delay_ms(1);
	}
}
void lcdcmd(char cmd)
{
	databus = (cmd & 0xF0);  // creating a nibble
	control &=~(1<<rs | 1<<rw);	// setting register select and read write pin to 0, set register select in command mode and read write pin to read mode
	control |= 1<<en;		// setting enable to 1
	_delay_ms(5);
	control &= ~(1<<en);	// setting enable to 0
	databus = ((cmd<<4) & 0xF0);	
	control &=~(1<<rs);	// setting register select and read write pin to 0, set register select in command mode and read write pin to read mode
	control &=~(1<<rw);
	control |= 1<<en;	// setting enable to 1
	_delay_ms(5);
	 control &= ~(1<<en);	// setting enable to 0
	_delay_ms(2);	
}
void main()
{
	DDRB |= 0x07;
	DDRD |= 0xF0;
	DDRD &= ~(1 << PD2);
	DDRD &= ~(1 << PD3);
	DDRC &= ~(1 << PC0);
	DDRB &= ~(1 << PB4);
	PORTD |= (1 << PD3);
	PORTC |= (1 << PC0);
	PORTD |= (1 << PD2);
	PORTB |= (1 << PB4); 
	DDRB |= (1 << PB5);
	DDRB |= (1 << PB3);
	int s;
	_delay_ms(15);
	lcdcmd(0x01);		//Clearing lcd
	lcdcmd(0x20);		//
	lcdcmd(0x28);		//Enable Display/Curso
	lcdcmd(0x0c);		//Clear and Home
	lcdcmd(0x06);		//Set Cursor Move Direction
	lcdcmd(0x80);		//Moves cursor to first address on the left of LINE 1
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT8);
	PCICR |= (1 << PCIE0);
	PCMSK0 |= (1 << PCINT4);
	EICRA |= (1 << ISC01);
	EICRA |= (1 << ISC11);
	EIMSK |= (1 << INT0)|(1 << INT1);
	TCCR1B |= 1 << WGM12;	//Setting up CTC mode
	OCR1A = 3125; // Setting timer ticks
	TCNT1 = 0;
	sei();	//Enable global interrupt

	while(1)
	{
		printstring(iter1);
		lcdcmd(0xC0);
		printstring(iter2);
		lcdcmd(0x01);
	}
}
ISR(INT0_vect)
{
	if (((PIND & (1 << PD2)) <= 0)) 
	{
		iter1++;
		TCNT1 = 0;
		TCCR1B |= (1 << CS12)|(1 << CS10);
		TIMSK1 |= (1 << OCIE1A); 
		EIMSK &= ~((1 << INT0)| (1 << INT1));
		PCICR &= ~(1 << PCIE1);
		PCICR &= ~(1 << PCIE0);

	}
}
ISR(TIMER1_COMPA_vect)
{
	TIMSK1 &= ~(1 << OCIE1A);
	TCCR1B &= ~((1 << CS12)|(1 << CS10));
	EIMSK |= ((1 << INT0)|(1 << INT1));
	PCICR |= (1 << PCIE1);
	PCICR |= (1 << PCIE0);
}
ISR(INT1_vect)
{
	if((PIND & (1 << PD3)) == 0)
	{
		iter2--;
		TCNT1 = 0;
		TCCR1B |= (1 << CS12)|(1 << CS10);
		TIMSK1 |= (1 << OCIE1A); 
		EIMSK &= ~((1 << INT0)| (1 << INT1));
		PCICR &= ~(1 << PCIE1);
		PCICR &= ~(1 << PCIE0);
	}
}
ISR(PCINT1_vect)
{
	if((PINC & (1 << PC0)) == 0)
	{
		PORTB ^= (1 << PB5);
		TCNT1 = 0;
		TCCR1B |= (1 << CS12)|(1 << CS10);
		TIMSK1 |= (1 << OCIE1A);
		EIMSK &= ~((1 << INT0)|(1 << INT1));
		PCICR &= ~(1 << PCIE1);
		PCICR &= ~(1 << PCIE0);
	}
}
ISR(PCINT0_vect)
{
	if((PINB & (1 << PB4)) == 0)
	{
		PORTB ^= (1 << PB3);
		TCNT1 = 0;
		TCCR1B |= (1 << CS12)|(1 << CS10);
		TIMSK1 |= (1 << OCIE1A);
		EIMSK &= ~((1 << INT0)|(1 << INT1));
		PCICR &= ~(1 << PCIE1);
		PCICR &= ~(1 << PCIE0);
	}
}