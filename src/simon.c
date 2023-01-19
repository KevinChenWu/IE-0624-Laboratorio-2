#include <avr/interrupt.h>

typedef enum {IDLE, INIT, GAME, FAIL} states;

ISR(PCINT1_vect) {
	if (PINA) {
		PORTB = 0xFF;
		PORTD |= 0x40;
	} else {
		PORTB = 0x00;
		PORTD &= 0xBF;
	}
}

ISR(PCINT2_vect) {
	if (PIND & (1<<PIND5 | 1<<PIND4 | 1<<PIND3 | 1<<PIND2 | 1<<PIND1 | 1<<PIND0)) {
		PORTB = 0xFF;
		PORTD |= 0x40;
	} else {
		PORTB = 0x00;
		PORTD &= 0xBF;
	}
}

int main(void) {
	DDRA = 0x00;
	PORTA = 0x00;
	DDRB = 0xFF;
	PORTB = 0x00;
	DDRD = 0x40;
	PORTD = 0x00;

	GIMSK = 0x18;
	PCMSK1 = 0x07;
	PCMSK2 = 0x3F;
	sei();

	states state = IDLE;

	while (1)
	{
		// switch (state)
		// {
		// case IDLE:
		// 	DDRA = 0x00;
		// 	PORTA = 0x00;
		// 	DDRB = 0xFF;
		// 	PORTB = 0x00;
		// 	DDRD = 0x40;
		// 	PORTD = 0x00;
		// 	break;
		// case INIT:
		// 	PORTB = 0xFF;
		// 	PORTD = 0x40;
		// 	break;
		// case FAIL:
		// 	break;
		// default:
		// 	break;
		// }
	}
	
}