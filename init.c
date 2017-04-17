#include <avr/io.h>
#include "modbus.h"
#include "main.h"
#include "system.h"

void mem_init()
{
	tmp = 0;
	tmpl = 0;
	tmpc = 0;
}

void t_sys_init()
{
	t_ac = T_MS_AC|T_10MS_AC|T_50MS_AC|T_S_AC|T_M_AC|T_H_AC;
	//Таймеры системные
	t_ms = 0;
	t_10ms = 0;
	t_50ms = 0;
	t_s = 0;
	t_m = 0;
	t_h = 0;

	//Таймеры остальные
//	t_pwm = 0;
	timer_init(&t_ain);

	OCR2 = 124;
	TCNT2 = 0;
	TIMSK |= (1<<OCIE2);
	TCCR2 = 0b11;
}

//*************Вход для датчиков NTC 10k*************
void ain_init()
{
	DDRF &= 0b11000000;
	PINF &= 0b11000000;
	PORTF &= 0b11000000;

	for (i = 0; i < 10; i++)
	{
		ainx[i] = 0;
	}

	ain[0] = 0;
	ain[1] = 0;
	ain[2] = 0;
	ain[3] = 0;
	ain[4] = 0;
	ain[5] = 0;

	//таблица для датчиков
	nc10k_r[0] = 867;
	nc10k_r[1] = 783;
	nc10k_r[2] = 681;
	nc10k_r[3] = 568;
	nc10k_r[4] = 512;
	nc10k_r[5] = 457;
	nc10k_r[6] = 355;
	nc10k_r[7] = 271;
	nc10k_r[8] = 204;
	nc10k_r[9] = 152;
	nc10k_r[10] = 114;

	nc10k_t[0] = -100;
	nc10k_t[1] = 0;
	nc10k_t[2] = 100;
	nc10k_t[3] = 200;
	nc10k_t[4] = 250;
	nc10k_t[5] = 300;
	nc10k_t[6] = 400;
	nc10k_t[7] = 500;
	nc10k_t[8] = 600;
	nc10k_t[9] = 700;
	nc10k_t[10] = 800;

	ain_cnt = 0;
	ain_sel = 0;
	ADMUX = ain_sel;
	ADCSRA = 0b10011111;
	ADCSRB = 0;

	timer_set(&t_ain, 20);
	timer_start(&t_ain);
}

//***************************************************

void aout_init()
{
	DDRB |= 0b00010000;
	DDRE |= 0b00111000;
	PORTB &= 0b11101111;
	PORTE &= 0b11000111;
	
	aout0 = 0;
	aout1 = 0;
	aout2 = 0;
	aout3 = 0;

	OCR3A = 0;
	OCR3B = 0;
	OCR3C = 0;
	TCNT3 = 0;
	TCCR3A = 0b10101011;
	TCCR3B = 0b00001001;
	TCCR3C = 0;

	OCR0 = 0;
	TCNT0 = 0;
	TCCR0 = 0b01101001;
}

void dio_init()
{
	din = 0;
	din_tmp = 0;
	dout = 0;
	led = 0;

	for (i = 0; i < 10; i++)
		t_din[i] = 0;
	
	DDRB |= 0b00001101;
	DDRE |= 0b11000100;

	DDRA = 0;
	DDRC &= 0b00111111;
}

