#include <avr/interrupt.h>

typedef enum {IDLE, INIT, INIT_LED_ON, INIT_LED_OFF, GAME, FAIL} states;

volatile states state = IDLE;
volatile unsigned short init_led_done = 0;

ISR(PCINT1_vect) {
	if (state == IDLE) {
		state = INIT;
	}
}

ISR(PCINT2_vect) {
	if (state == IDLE) {
		state = INIT;
	}
}

ISR(TIMER0_OVF_vect) {
	if (state == INIT_LED_ON) {
		state = INIT_LED_OFF;
	} else if (state == INIT_LED_OFF) {
		if (!init_led_done) {
			state = INIT_LED_ON;
			init_led_done = 1;
		} else {
			state = GAME;
		}
	}
}

void lfsr16(unsigned short *rnd_number);

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

	TCCR0A = 0x00;
	TCCR0B = 0x05;
	TIMSK = 0x02;

	unsigned short rnd_number = 1;
	unsigned short *ptr_rnd = &rnd_number;

	unsigned short leds_position[9] = {0};
	unsigned short leds_number = 4;
	unsigned short n = 0;


	while (1)
	{
		switch (state) {
		case IDLE:
			DDRA = 0x00;
			PORTA = 0x00;
			DDRB = 0xFF;
			PORTB = 0x00;
			DDRD = 0x40;
			PORTD = 0x00;
			break;
		case INIT:
			if (n < leds_number) {
				leds_position[n] = rnd_number % leds_number;
				n++;
			} else {
				state = INIT_LED_ON;
				TCNT0 = 0;
			}
			break;
		case INIT_LED_ON:
			PORTB |= 0xFF;
			PORTD |= 0x40;
			break;
		case INIT_LED_OFF:
			PORTB &= 0x00;
			PORTD &= 0xBF;
			break;
		case FAIL:
			break;
		default:
			break;
		}
		lfsr16(ptr_rnd);
	}
	
}

void lfsr16(unsigned short *rnd_number) {
	// Se pregunta si el último bit es 1
	if ((*rnd_number) & 1) {
		// Se desplaza el número un bit a la derecha
		(*rnd_number) >>= 1;
		// Se hace un XOR del número con la máscara y
		// se actualiza a sí mismo
		(*rnd_number) ^= (1<<15) + (1<<14) + (1<<12) + (1<<3);
	}
	else {
		// Se desplaza el número un bit a la derecha
		(*rnd_number) >>= 1;
	}
}