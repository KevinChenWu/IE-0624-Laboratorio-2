#include <avr/interrupt.h>

typedef enum {
	IDLE,
	INIT,
	INIT_LED_ON,
	INIT_LED_OFF,
	GAME_SETUP,
	GAME_LED,
	WAIT_USER,
	VERIFY,
	WIN,
	LOSE,
	LOSE_LED_ON,
	LOSE_LED_OFF
} states;

volatile states state = IDLE;
volatile unsigned short init_led_done = 0;
volatile unsigned short game_led_done = 0;
volatile unsigned short game_led_count = 0;
volatile unsigned short lose_led_done = 0;
volatile unsigned short lose_led_count = 0;
volatile unsigned short mem_leds = 4;
volatile unsigned short button;

ISR(INT0_vect) {
	if (state == IDLE) {
		state = INIT;
	} else if (state == WAIT_USER) {
		button = 1;
		state = VERIFY;
	}
	
}

ISR(INT1_vect) {
	if (state == IDLE) {
		state = INIT;
	} else if (state == WAIT_USER) {
		button = 2;
		state = VERIFY;
	}
}

ISR(PCINT0_vect) {
	if (state == IDLE) {
		state = INIT;
	} else if (state == WAIT_USER) {
		button = 3;
		state = VERIFY;
	}
}

ISR(PCINT1_vect) {
	if (state == IDLE) {
		state = INIT;
	} else if (state == WAIT_USER) {
		button = 0;
		state = VERIFY;
	}
}

ISR(TIMER0_OVF_vect) {
	if (state == INIT_LED_ON) {
		state = INIT_LED_OFF;
		PORTB &= 0x00;
	} else if (state == INIT_LED_OFF) {
		if (!init_led_done) {
			state = INIT_LED_ON;
			PORTB |= 0xF0;
			init_led_done = 1;
		} else {
			state = GAME_SETUP;
		}
	} else if (state == LOSE_LED_ON) {
		state = LOSE_LED_OFF;
		PORTB &= 0x00;
	} else if (state == LOSE_LED_OFF) {
		if (!lose_led_done) {
			if (!lose_led_count) {
				lose_led_count = 1;
			} else {
				lose_led_count = 2;
				lose_led_done = 1;
			}
			state = LOSE_LED_ON;
			PORTB |= 0xF0;
		} else {
			state = IDLE;
		}
	}
}

ISR(TIMER1_COMPA_vect) {
	if (state == GAME_LED) {
		if (game_led_count < (mem_leds-1)) {
			game_led_count++;
		} else if (game_led_count == (mem_leds-1)) {
			state = WAIT_USER;
			game_led_done = 1;
			game_led_count = 0;
		}
	}
	
}

void lfsr16(unsigned short *rnd_number);

int main(void) {
	DDRA = 0x06;
	PORTA = 0x00;
	DDRB = 0xFE;
	PORTB = 0x00;
	DDRD = 0x73;
	PORTD = 0x00;

	MCUCR = 0x85;
	GIMSK = 0xE8;
	PCMSK = 0x01;
	PCMSK1 = 0x01;
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
	unsigned short delay;


	while (1)
	{
		switch (state) {
		case IDLE:
			DDRA = 0x06;
			PORTA = 0x00;
			DDRB = 0xFE;
			PORTB = 0x00;
			DDRD = 0x63;
			PORTD = 0x00;

			n = 0;
			init_led_done = 0;
			game_led_done = 0;
			game_led_count = 0;
			lose_led_done = 0;
			lose_led_count = 0;
			mem_leds = 4;
			break;
		case INIT:
			state = INIT_LED_ON;
			PORTB |= 0xF0;
			TCNT0 = 0;
			break;
		case INIT_LED_ON:
			break;
		case INIT_LED_OFF:
			break;
		case GAME_SETUP:
			if (n < mem_leds) {
				game_leds[n] = rnd_number % 4;
				n++;
			} else {
				state = GAME_LED;
				TCNT1H = 0;
				TCNT1L = 0;
			}
			break;
		case GAME_LED:
			if (!game_led_done) {
				led = game_leds[game_led_count];
				switch (led) {
				case 0:
					PORTB = 0x80;
					break;
				case 1:
					PORTB = 0x40;
					break;
				case 2:
					PORTB = 0x20;
					break;
				case 3:
					PORTB = 0x10;
					break;
				}
			} else {
				PORTB = 0x00;
				game_led_count = 0;
			}
			break;
		case WAIT_USER:
			PORTB = 0x00;
			break;
		case VERIFY:
			led = game_leds[game_led_count];
			if (button == led) {
				if (game_led_count == (mem_leds-1)) {
					state = WIN;
				} else {
					state = WAIT_USER;
					game_led_count++;
				}
			} else {
				state = LOSE;
			}
			break;
		case WIN:
			n = 0;
			game_led_done = 0;
			game_led_count = 0;
			lose_led_done = 0;
			lose_led_count = 0;
			if (mem_leds < 9) {
				mem_leds++;
				delay = (OCR1AH << 8) | (OCR1AL);
				delay -= 195;
				OCR1AL = delay;
				OCR1AH = (delay >> 8);
			}
			state = GAME_SETUP;
			break;
		case LOSE:
			state = LOSE_LED_ON;
			PORTB |= 0xF0;
			TCNT0 = 0;
			break;
		case LOSE_LED_ON:
			break;
		case LOSE_LED_OFF:
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