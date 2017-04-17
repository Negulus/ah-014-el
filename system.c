#include <avr/io.h>
#include <avr/interrupt.h>
#include "main.h"
#include "modbus.h"
#include "system.h"
#include "panel.h"

void s_heat();
void s_vent();

void s_init()
{
	s_state = 0;
	s_er = 0;
	speed_last = 0;

	//Конфигурация панели
	p_menu_line_en[0] = 0b11111;
	p_menu_line_en[1] = 0b1101;
	p_menu_line_en[2] = 0b11;
	p_mode_en = MODE_WINTER|MODE_SUMMER;
	p_speed_en = 3;
	p_inout_en = SENS_AIR|SENS_LOC|OUT_HEAT;

	//Чтение памяти
	s_mem_read();

	pid_init(pid_p, pid_i, 0, 0, 100000, 100000, &pid_heat);
	heat_p = 0;

	timer_init(&t_dump);
	timer_init(&t_speed);
	timer_init(&t_blow);
	timer_set(&t_dump, s_dump);
	timer_set(&t_speed, 20);
	timer_set(&t_blow, s_blow);
	t_pwm = 0;
}

void s_mem_read()
{
	//Чтение памяти
	pid_p = eeprom_read_word(&p_menu_2[0]);
	if (pid_p < 1)
	{
		pid_p = 1;
		eeprom_write_word(&p_menu_2[0], pid_p);
	} else if (pid_p > 200)
	{
		pid_p = 200;
		eeprom_write_word(&p_menu_2[0], pid_p);
	}

	pid_i = eeprom_read_word(&p_menu_2[1]);
	if (pid_i < 1)
	{
		pid_i = 1;
		eeprom_write_word(&p_menu_2[1], pid_i);
	} else if (pid_i > 200)
	{
		pid_i = 200;
		eeprom_write_word(&p_menu_2[1], pid_i);
	}


	s_blow = eeprom_read_word(&p_menu_1[2]);
	if (s_blow < 0)
	{
		s_blow = 0;
		eeprom_write_word(&p_menu_1[2], s_blow);
	} else if (s_blow > 30)
	{
		s_blow = 30;
		eeprom_write_word(&p_menu_1[2], s_blow);
	}


	s_dump = eeprom_read_word(&p_menu_1[3]);
	if (s_dump < 0)
	{
		s_dump = 0;
		eeprom_write_word(&p_menu_1[3], s_dump);
	} else if (s_dump > 60)
	{
		s_dump = 60;
		eeprom_write_word(&p_menu_1[3], s_dump);
	}

	s_pwm = eeprom_read_word(&p_menu_1[0]);
	if (s_pwm < 1)
	{
		s_pwm = 1;
		eeprom_write_word(&p_menu_1[0], s_pwm);
	} else if (s_pwm > 60)
	{
		s_pwm = 60;
		eeprom_write_word(&p_menu_1[0], s_pwm);
	}
}

void s_timer_ms()
{
}

void s_timer_10ms()
{
	t_pwm++;
	if (t_pwm >= (s_pwm * 100))
		t_pwm = 0;

	if (s_pwm_tmp == 0)
	{
		asm volatile ("nop");
		aout0 = 0;
		asm volatile ("nop");
	}
	else if (s_pwm_tmp >= t_pwm)
		aout0 = 100;
	else
		aout0 = 0;
}

void s_timer_50ms()
{
	timer_upd(&t_speed);

	s_check();
	s_heat();

	if (heat_p > 50)
		s_pwm_tmp = (heat_p - 50) * 2 * s_pwm;
	else
		s_pwm_tmp = heat_p * 2 * s_pwm;

	dout &= DOUT_EN;
}

void s_timer_s()
{
	//Чтение памяти
	s_mem_read();

	pid_heat.p = pid_p;
	pid_heat.i = pid_i;

	timer_set(&t_dump, s_dump);
	timer_set(&t_speed, 20);
	timer_set(&t_blow, s_blow);

	timer_upd(&t_dump);
	timer_upd(&t_blow);

	pid_calc(p_set_t, ain[0], &pid_heat);
}

void s_heat()
{
	if (s_state & SYS_BLOCK)
	{
		s_state &= ~(SYS_START|SYS_BLOW);
	}

	if (s_state & SYS_START)
	{
		s_state |= SYS_WORK;
		s_state |= SYS_DUMP;
		timer_start(&t_dump);
//		led |= LED_GREEN;

		if (timer_out(&t_dump))
		{			
			s_state |= SYS_VENT;

			if (p_mode & MODE_WINTER)
			{
				if (s_er & SYS_ER_OVERHEAT)
				{
					s_state &= ~SYS_HEAT2;
					heat_p = 0;
				}
				else
				{
					heat_p = pid_heat.out / 1000;
					if (heat_p > 50)
						s_state |= SYS_HEAT2;
					else
						s_state &= ~SYS_HEAT2;
				}

				s_state |= SYS_HEAT;
				s_state |= SYS_BLOW;
			}
			else
			{
				s_state &= ~SYS_HEAT;
				s_state &= ~SYS_HEAT2;
				s_state &= ~SYS_BLOW;
				pid_reset(&pid_heat);
				heat_p = 0;
			}
		}
	}
	else if (s_state & SYS_BLOW)
	{
		s_state &= ~SYS_HEAT;
		s_state &= ~SYS_HEAT2;
		heat_p = 0;
		timer_start(&t_blow);
		if (timer_out(&t_blow))
		{
			s_state &= ~SYS_BLOW;
		}
	}
	else
	{
		s_state &= ~SYS_WORK;
		s_state &= ~SYS_DUMP;
		s_state &= ~SYS_VENT;
		s_state &= ~SYS_HEAT;
		s_state &= ~SYS_HEAT2;
		s_state &= ~SYS_BLOW;
		timer_stop(&t_dump);
		timer_stop(&t_speed);
		timer_stop(&t_blow);
		timer_reset(&t_dump);
		timer_reset(&t_speed);
		timer_reset(&t_blow);
		pid_reset(&pid_heat);
		heat_p = 0;
//		led &= ~LED_GREEN;
	}

	(s_state & SYS_DUMP) ? (dout |= DOUT_DUMP) : (dout &= ~DOUT_DUMP);
	(s_state & SYS_HEAT) ? (dout |= DOUT_HEAT) : (dout &= ~DOUT_HEAT);
	(s_state & SYS_HEAT2) ? (dout |= DOUT_HEAT2) : (dout &= ~DOUT_HEAT2);
	(s_state & SYS_ALARM) ? (led |= LED_RED) : (led &= ~LED_RED);

	if (s_state & SYS_VENT)
	{
		if (din & DIN_FILTER)
			s_er |= SYS_ER_FILTER;
		else
			s_er &= ~SYS_ER_FILTER;

		if (p_speed != speed_last)
		{
			timer_reset(&t_speed);
			speed_last = p_speed;
		}

		timer_start(&t_speed);
		if (timer_out(&t_speed))
		{
			switch (p_speed)
			{
			case 1: dout |= DOUT_SPEED1;
			break;
			case 2: dout |= DOUT_SPEED2;
			break;
			case 3: dout |= DOUT_SPEED3;
			break;
			}
		}
		else
			dout &= ~(DOUT_SPEED1|DOUT_SPEED2|DOUT_SPEED3);
	}
	else
		dout &= ~(DOUT_SPEED1|DOUT_SPEED2|DOUT_SPEED3);
}

void s_check()
{
	if (din & DIN_FIRE)
		s_er |= SYS_ER_FIRE;
	else
		s_er &= ~SYS_ER_FIRE;

	if (din & DIN_VENT)
		s_er |= SYS_ER_VENT1;
	else
		s_er &= ~SYS_ER_VENT1;

	if (din & DIN_OVERHEAT)
		s_er |= SYS_ER_OVERHEAT;
	else
		s_er &= ~SYS_ER_OVERHEAT;

	if (s_er & (SYS_ER_FIRE|SYS_ER_VENT1|SYS_ER_VENT2|SYS_ER_DPS1|SYS_ER_DPS2|SYS_ER_FROST|SYS_ER_AIN))
		s_state |= SYS_BLOCK;
	else
		s_state &= ~SYS_BLOCK;

	if (s_er & ~(SYS_ER_FILTER))
		s_state |= SYS_ALARM;
	else
		s_state &= ~SYS_ALARM;

	if (!(p_status & (1<<P_STAT_TIMER)))
	{
		if (p_status & (1<<P_STAT_RUN))
			s_state |= SYS_START;
		else
			s_state &= ~SYS_START;
	}
}
