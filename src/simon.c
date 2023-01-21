#include <avr/interrupt.h>

typedef enum {
	IDLE,
	INIT,
	LED_ON,
	LED_OFF,
	GAME_SETUP,
	GAME_LED,
	WAIT_USER,
	VERIFY,
	WIN,
	LOSE
} states;

volatile states state = IDLE;
volatile unsigned short init_led_done = 0;
volatile unsigned short game_led_count = 0;
volatile unsigned short game_leds = 4;
volatile unsigned short button;

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
	if (state == LED_ON) {
		state = LED_OFF;
		PORTB &= 0x00;
	} else if (state == LED_OFF) {
		if (!init_led_done) {
			state = LED_ON;
			PORTB |= 0xF0;
			init_led_done = 1;
		} else {
			state = GAME_SETUP;
		}
	}
}

ISR(TIMER1_COMPA_vect) {
	if (state == GAME_LED) {
		if (game_led_count < (game_leds-1)) {
			game_led_count++;
		} else {
			state = WAIT_USER;
			PORTB = 0x00;
			PORTD = 0x00;
		}
	}
	
}

void lfsr16(unsigned short *rnd_number);

int main(void) {
	DDRA = 0x06;
	PORTA = 0x00;
	DDRB = 0xFF;
	PORTB = 0x00;
	DDRD = 0x63;
	PORTD = 0x00;

	GIMSK = 0xD8;
	PCMSK1 = 0x01;
	PCMSK2 = 0x10;
	sei();

	TCCR0A = 0x00;
	TCCR0B = 0x05;

	TCCR1A = 0x00;
	TCCR1B = 0x0D;
	TCCR1C = 0x00;

	OCR1AH = 0x07;
	OCR1AL = 0xA1;

	TIMSK = 0x42;

	unsigned short rnd_number = 1;
	unsigned short *ptr_rnd = &rnd_number;

	unsigned short game_leds[9];
	unsigned short n = 0;
	unsigned short led;


	while (1)
	{
		switch (state) {
		case IDLE:
			DDRA = 0x06;
			PORTA = 0x00;
			DDRB = 0xFF;
			PORTB = 0x00;
			DDRD = 0x63;
			PORTD = 0x00;
			break;
		case INIT:
			state = LED_ON;
			PORTB |= 0xF0;
			TCNT0 = 0;
			break;
		case LED_ON:
			break;
		case LED_OFF:
			break;
		case GAME_SETUP:
			if (n_leds < game_leds) {
				leds_position[n_leds] = rnd_number % game_leds;
				n_leds++;
			} else {
				state = GAME_LED;
				TCNT1H = 0;
				TCNT1L = 0;
			}
		case GAME_LED:
			led = leds_position[game_led_count];
			switch (led) {
			case 0:
				PORTB = 0x80;
				PORTD = 0x00;
				break;
			case 1:
				PORTB = 0x40;
				PORTD = 0x00;
				break;
			case 2:
				PORTB = 0x20;
				PORTD = 0x00;
				break;
			case 3:
				PORTB = 0x10;
				PORTD = 0x00;
				break;
			case 4:
				PORTB = 0x08;
				PORTD = 0x00;
				break;
			case 5:
				PORTB = 0x04;
				PORTD = 0x00;
				break;
			case 6:
				PORTB = 0x02;
				PORTD = 0x00;
				break;
			case 7:
				PORTB = 0x01;
				PORTD = 0x00;
				break;
			case 8:
				PORTB = 0x00;
				PORTD = 0x40;
				break;
			default:
				PORTB = 0x00;
				PORTD = 0x00;
				break;
			}
		case WAIT_USER:
			break;
		case VERIFY:
			// led = leds_position[game_led_count];
			// if (button == led) {
			// 	if (game_led_count == (game_leds-1)) {
			// 		state = WIN;
			// 		game_led_count = 0;
			// 	} else {
			// 		state = WAIT_USER;
			// 		game_led_count++;
			// 	}
			// } else {
			// 	state = LOSE;
			// 	game_led_count = 0;
			// }
			break;
		case WIN:
			PORTB = 0xF0;
			break;
		case LOSE:
			PORTB = 0x0F;
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