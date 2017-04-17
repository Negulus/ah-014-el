/*
0 - работа системы
1 - работа 1
2 - работа 2
3 - авария 1
4 - авария 2
5 - включение резерва
6 - пожарная авария

10 - уставка
11 - температура комнаты
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include "modbus.h"
#include "main.h"
#include "sd.h"
#include "pid.h"
#include "panel.h"
#include "system.h"
#include "twi.h"

void ain_test();

int main (void)
{
	DDRA = 0;
	DDRB = 0;
	DDRC = 0;
	DDRD = 0;
	DDRE = 0;
	DDRF = 0;
	DDRG = 0;
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTE = 0;
	PORTF = 0;
	PORTG = 0;

	DDRF |= 0b11000000;
	DDRB |= 0b01100000;

	mem_init();
	dio_init();
	t_sys_init();
	mb_init(&mb0, MB_MASTER, &UCSR0A, &UCSR0B, &UCSR0C, &UBRR0L, &UBRR0H, &UDR0, &DDRB, &PORTB, 5, MB_SB_1, MB_PR_NONE);
//	mb_init(&mb1, MB_SLAVE, &UCSR1A, &UCSR1B, &UCSR1C, &UBRR1L, &UBRR1H, &UDR1, &DDRB, &PORTB, 6, MB_SB_1, MB_PR_EVEN);
	twi_init();
	ain_init();
	aout_init();
	s_init();
	p_init(&mb0);
	sd_init(&mb1);

	//определение регистров
	sei();

	PORTF |= (1<<6);

	led |= LED_GREEN;

    while (1)
    {
		asm volatile ("nop");

		if ((p_mb->state == 0) && (p_mb->accept == 1))
		{
			p_mb->accept = 0;
			timer_mb_next_set(p_mb, T_MB_NEXT);
			p_queue();

			asm volatile ("nop");
		}
		
		asm volatile ("nop");		
    }
    return 0;
} 

//Таймер
ISR (TIMER2_COMP_vect)
{
	TCNT2 = 0;
	//1 милисекунда
	t_ms++;
	t_10ms++;
	t_50ms++;

	if (t_ac & T_MS_AC)
	{
		t_ac &= ~T_MS_AC;
		timer_ms();
		t_ac |= T_MS_AC;
	}

	if (t_10ms >= 10)
	{
		t_10ms = 0;
		if (t_ac & T_10MS_AC)
		{
			t_ac &= ~T_10MS_AC;
			timer_10ms();
			t_ac |= T_10MS_AC;
		}
	}

	if (t_50ms >= 50)
	{
		t_50ms = 0;
		if (t_ac & T_50MS_AC)
		{
			t_ac &= ~T_50MS_AC;
			timer_50ms();
			t_ac |= T_50MS_AC;
		}
	}

	if (t_ms >= 1000)
	{
		t_ms = 0;
		//1 секунда
		t_s++;
		if (t_ac & T_S_AC)
		{
			t_ac &= ~T_S_AC;
			timer_s();
			t_ac |= T_S_AC;
		}

		led &= ~LED_GREEN;

		if (t_s >= 60)
		{
			t_s = 0;
			//1 минута
			t_m++;
			if (t_ac & T_M_AC)
			{
				t_ac &= ~T_M_AC;
				timer_m();
				t_ac |= T_M_AC;
			}

			if (t_m >= 60)
			{
				t_m = 0;
				//1 час
				if (t_ac & T_H_AC)
				{
					t_ac &= ~T_H_AC;
					timer_h();
					t_ac |= T_H_AC;
				}
			}
		}
	}

	return;
}

void timer_ms()
{
	//Таймеры ModBus
	timer_mb(&mb0);
	timer_mb_accept(&mb0);
	timer_mb_next(&mb0);
	timer_mb_check(&mb0);
//	timer_mb(&mb1);
//	timer_mb_accept(&mb1);
//	timer_mb_next(&mb1);
//	timer_mb_check(&mb1);

	timer_upd(&t_ain);

	if (twi.state == 0)
		t_twi++;

	if (t_twi > t_twi_set)
	{
		twi.accept = 1;
		twi_queue();
		t_twi = 0;
	}

	timer_io_check();

	s_timer_ms();

//****Отправка ответа диспетчеризации, если раньше было занято****
	if (sd_state == (1<<1))
	{
		sd_state = 0;
		sd_ans();
		mb_start_tx(sd_mb);
	}
//****************************************************************

	//Работа аналоговых входов
	if (timer_out(&t_ain))
	{
		//запуск ацп
		ADCSRA |= (1<<ADEN);
		ADCSRA |= 0x40;

		timer_stop(&t_ain);
		timer_reset(&t_ain);
	}
}

void timer_10ms()
{
	s_timer_10ms();
}

void timer_50ms()
{
	//Обновление системы
	s_timer_50ms();

	//Обновление системы диспетчеризации
//	sd_update();

//	if (t_pwm > 99)
//		t_pwm = 0;
//	else
//		t_pwm++;

//	if (t_pwm >= aout1)
//		aout0 = 0;
//	else
//		aout0 = 100;

	//Обновление дискретных входов и выходов
	io_check();

}

void timer_s()
{
	shed_upd();
	shedule();

	s_timer_s();
}

void timer_m()
{
	return;
}

void timer_h()
{
	return;
}

//*************Вход для датчиков NTC 10k*************
ISR (ADC_vect)
{	
	for (i = 9; i > 0; i--)
		ainx[i] = ainx[i-1];
	ainx[0] = ADC;
	
	ain_cnt++;

	if (ain_cnt >= 10)
	{

		for (int i = 1; i < 10; i++)
		{
			ainx[0] += ainx[i];
		}
		ainx[0] /= 10;	

		if (ain_sel == 5)
		{
			ain[ain_sel] = ainx[0];
		}
		else
		{
			ain[ain_sel] = ain_calc(ainx[0]);
		}

		if (ain_sel >= 5)
			ain_sel = 0;
		else
			ain_sel++;

		ADMUX = ain_sel;
		ain_cnt = 0;
	}

	timer_reset(&t_ain);
	timer_start(&t_ain);
}
//***************************************************

void io_check()
{

	//Дискретные выходы
	if ((dout&(1<<0)) > 0)
		PORTE |= (1<<2);
	else
		PORTE &= ~(1<<2);

	if ((dout&(1<<1)) > 0)
		PORTE |= (1<<6);
	else
		PORTE &= ~(1<<6);

	if ((dout&(1<<2)) > 0)
		PORTE |= (1<<7);
	else
		PORTE &= ~(1<<7);

	if ((dout&(1<<3)) > 0)
		PORTB |= (1<<0);
	else
		PORTB &= ~(1<<0);

	if ((dout&(1<<4)) > 0)
		PORTB |= (1<<2);
	else
		PORTB &= ~(1<<2);

	if ((dout&(1<<5)) > 0)
		PORTB |= (1<<3);
	else
		PORTB &= ~(1<<3);

	if (led & LED_GREEN)
		PORTF |= (1<<6);
	else
		PORTF &= ~(1<<6);

	if (led & LED_RED)
		PORTF |= (1<<7);
	else
		PORTF &= ~(1<<7);

	//Дискретные входы
	din_any(PINA, 0, 0, NC);
	din_any(PINA, 1, 1, NC);
	din_any(PINA, 2, 2, NC);
	din_any(PINA, 3, 3, NC);
	din_any(PINA, 4, 4, NC);
	din_any(PINA, 5, 5, NC);
	din_any(PINA, 6, 6, NC);
	din_any(PINA, 7, 7, NC);
	din_any(PINC, 7, 8, NC);
	din_any(PINC, 6, 9, NC);

	//Аналоговые выходы
	//AOUT0
	tmpl = aout0 * 10;
	if (tmpl > 1023)
		tmpl = 1023;
	else if (tmpl < 0)
		tmpl = 0;
	if (tmpl == 0)
	{
		TCCR3A &= ~(1<<COM3A1);
		PORTE &= ~(1<<3);
	}
	else
	{
		TCCR3A |= (1<<COM3A1);
		OCR3A = (int)tmpl;
	}

	//AOUT1
	tmpl = aout1 * 10;
	if (tmpl > 1023)
		tmpl = 1023;
	else if (tmpl < 0)
		tmpl = 0;
	if (tmpl == 0)
	{
		TCCR3A &= ~(1<<COM3B1);
		PORTE &= ~(1<<4);
	}
	else
	{
		TCCR3A |= (1<<COM3B1);
		OCR3B = (int)tmpl;
	}

	//AOUT2
	tmpl = aout2 * 10;
	if (tmpl > 1023)
		tmpl = 1023;
	else if (tmpl < 0)
		tmpl = 0;
	if (tmpl == 0)
	{
		TCCR3A &= ~(1<<COM3C1);
		PORTE &= ~(1<<5);
	}
	else
	{
		TCCR3A |= (1<<COM3C1);
		OCR3C = (int)tmpl;
	}

	//AOUT3
	tmpl = aout3 * 255;
	tmpl /= 100;
	if (tmpl > 255)
		tmpl = 255;
	else if (tmpl < 0)
		tmpl = 0;
	if (tmpl == 0)
	{
		TCCR0 &= ~(1<<COM0);
		PORTB &= ~(1<<4);
	}
	else
	{
		TCCR0 |= (1<<COM0);
		OCR0 = (int)tmpl;
	}
}

//Табличный расчёт для аналогового входа NTC10k
int ain_calc(int aint)
{
	if (aint >= nc10k_r[0])
		aint = nc10k_t[0];
	else if (aint <= nc10k_r[10])
		aint = nc10k_t[10];
	else
	{
		for (i = 1; i < 11; i++)
		{
			if (aint >= nc10k_r[i])
			{
				aint = nc10k_r[i-1] - aint;
				aint *= 100;
				aint = aint / (nc10k_r[i-1] - nc10k_r[i]);
				aint = aint * (nc10k_t[i] - nc10k_t[i-1]);
				aint /= 100;
				aint = nc10k_t[i-1] + aint;
				
				break;
			}
		}
	}
	return aint;
}

//Проверка дискретного входа
void din_any(int port, char bit, int tmp_din, unsigned char nc)
{
	if (((port&(1<<bit)) == 0 && (din_tmp&(1<<tmp_din)) > 0) || ((port&(1<<bit)) > 0 && (din_tmp&(1<<tmp_din)) == 0))
	{
		if ((port&(1<<bit)) > 0)
			din_tmp |= (1<<tmp_din);
		else
			din_tmp &= ~(1<<tmp_din);
		t_din[tmp_din] = 0;
	}

	if (nc == NC)
	{
		if (t_din[tmp_din] >= T_DIN && ((din_tmp&(1<<tmp_din)) != (din&(1<<tmp_din))))
		{
			if ((din_tmp&(1<<tmp_din)) > 0)
				din |= (1<<tmp_din);
			else
				din &= ~(1<<tmp_din);
		}
	}
	else
	{
		if (t_din[tmp_din] >= T_DIN && ((din_tmp&(1<<tmp_din)) == (din&(1<<tmp_din))))
		{
			if ((din_tmp&(1<<tmp_din)) == 0)
				din |= (1<<tmp_din);
			else
				din &= ~(1<<tmp_din);
		}
	}
}

//*********************************************
//                   Таймеры
//*********************************************
void timer_mb_set(mb_data *mb, unsigned int set)
{
	mb->t = 0;
	mb->t_set = set;
}

void timer_mb_stoptx_set(mb_data *mb, unsigned int set)
{
	mb->t_stoptx = 0;
	mb->t_stoptx_set = set;
}

void timer_mb(mb_data *mb)
{
	if (mb->t_set > 0)
	{
		mb->t++;
		if (mb->t >= mb->t_set)
		{
			mb->t = 0;
			mb->t_set = 0;
			mb_stop_rx(mb);
		}
	}

	if (mb->t_stoptx_set > 0)
	{
		mb->t_stoptx++;
		if (mb->t_stoptx >= mb->t_stoptx_set)
		{
			mb->t_stoptx = 0;
			mb->t_stoptx_set = 0;
			mb_stop_tx(mb);
		}
	}
}

void timer_mb_check(mb_data *mb)
{
	mb->t_check++;

	if (mb->type == MB_SLAVE)
	{
		tmpc = 0;
		if ((mb->state & MB_RX) == 0)
			tmpc = 1;

		if (((*mb->uscrb & (1<<4)) == 0) || ((*mb->uscrb & (1<<7)) == 0))
			tmpc = 1;

		if (tmpc == 1)
			mb->t_check++;
		else
			mb->t_check = 0;
	}

	if (mb->t_check >= T_MB_CHECK)
	{
		mb->state = 0;
		if (mb->type == MB_SLAVE)
		{
			mb_start_rx(mb);
		}
		mb->t_check = 0;
	}
}

void timer_mb_accept(mb_data *mb)
{
	if (mb->type == MB_MASTER)
	{
		mb->t_accept++;
		if (mb->t_accept >= T_MB_ACCEPT)
		{
			mb->accept = 1;
			mb->t_accept = 0;
		}
	}
}

void timer_mb_next(mb_data *mb)
{
	if (mb->t_next_set > 0)
	{
		if (mb->t_next >= mb->t_next_set)
		{
			mb->t_next = 0;
			mb->t_next_set = 0;

			if ((mb->state & MB_TX) > 0)
			{
				mb_start_tx(mb);
			}
		}
		else
		{
			mb->t_next++;
		}
	}
}

void timer_mb_next_set(mb_data *mb, unsigned int set)
{
	mb->t_next = 0;
	mb->t_next_set = set;
}

//*********************************************

void timer_io_check()
{
	for (i = 0; i < 10; i++)
		if (t_din[i] <= T_DIN)
			t_din[i]++;
}

//Система таймеров
//Обновление показаний
void timer_upd(timer_data *timer)
{
	if ((timer->stat&(1<<0)) && !(timer->stat&(1<<1)) && (timer->set > 0))
	{
		if (timer->t < timer->set)
			timer->t++;
		else
			timer->stat |= (1<<1);
	}
}

//Проверка состояния
char timer_out(timer_data *timer)
{
	if (timer->stat&(1<<1) || timer->set == 0)
		return 1;
	else
		return 0;
}

//Обновление настроек
void timer_set(timer_data *timer, unsigned int set)
{
	timer->set = set;
}

//Запуск таймера
void timer_start(timer_data *timer)
{
	timer->stat |= (1<<0);
}

//Остановка таймера
void timer_stop(timer_data *timer)
{
	timer->stat &= ~(1<<0);
}

//Сброс таймера
void timer_reset(timer_data *timer)
{
	timer->stat &= ~(1<<1);
	timer->t = 0;
}

//Инициализация таймера
void timer_init(timer_data *timer)
{
	timer->stat = 0;
	timer->set = 0;
	timer->t = 0;
}
