#include <avr/io.h>
#include <avr/interrupt.h>
#include "modbus.h"
#include "main.h"
#include "pid.h"
#include "panel.h"
#include "system.h"
#include "twi.h"

unsigned int	p_menu_1[16] EEMEM;
unsigned int	p_menu_2[16] EEMEM;
unsigned int	p_day_0[12] EEMEM;
unsigned int	p_day_1[12] EEMEM;
unsigned int	p_day_2[12] EEMEM;
unsigned int	p_day_3[12] EEMEM;
unsigned int	p_day_4[12] EEMEM;
unsigned int	p_day_5[12] EEMEM;
unsigned int	p_day_6[12] EEMEM;

void p_init(mb_data *mb)
{
	p_line = P_LINE_AW;
	p_mb = mb;
}

//Очередь
void p_queue()
{
	if (p_line < 1)
		p_line = P_LINE_AW;

	switch (p_line)
	{
	case P_LINE_AR:
		p_line_ar();
		break;

	case P_LINE_AW:
		p_line_aw();
		break;

	case P_LINE_BR:
		p_line_br();
		break;

	case P_LINE_BW:
		p_line_bw();
		break;

	case P_LINE_CR:
		p_line_cr();
		break;

	case P_LINE_CW:
		p_line_cw();
		break;

	case P_LINE_STAT:
		p_line_stat();
		break;

	case P_LINE_MR:
		p_line_mr();
		break;

	case P_LINE_MW:
		p_line_mw();
		break;

	case P_LINE_MC:
		p_line_mc();
		break;

	case P_LINE_E0:
		p_line_e0();
		break;

	case P_LINE_E1:
		p_line_e1();
		break;

	case P_LINE_E2:
		p_line_e2();
		break;

	case P_LINE_E3:
		p_line_e3();
		break;

	case P_LINE_E4:
		p_line_e4();
		break;

	default:
		p_line = P_LINE_AW;
	}
}

//Параметры группы A

void p_line_ar()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->func = MB_FR;
		p_mb->reg_n = 0x0a00;
		p_mb->cnt = 6;
		reg_read(p_mb);
	}
	else
	{
		p_line = P_LINE_AW;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

void p_line_aw()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->reg_v[0] = p_menu_line_en[0];
		p_mb->reg_v[1] = p_menu_line_en[1];
		p_mb->reg_v[2] = p_menu_line_en[2];
		p_mb->reg_v[3] = p_mode_en;
		p_mb->reg_v[4] = p_speed_en;
		p_mb->reg_v[5] = p_inout_en;
		p_mb->func = MB_FW;
		p_mb->reg_n = 0x0a00;
		p_mb->cnt = 6;
		reg_mwrite(p_mb);
	}
	else
	{
		p_line = P_LINE_BR;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

//Параметры группы B
void p_line_br()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->func = MB_FR;
		p_mb->reg_n = 0x0b01;
		p_mb->cnt = 13;
		reg_read(p_mb);
	}
	else
	{
		p_mb->reg_n = 0x0b01;
		p_mb->cnt = 10;

		//Регистр B01
		if (!(p_mb->reg_v[0] & p_mode_en))
			p_mb->reg_v[0] = MODE_SUMMER;
		else if (s_state & SYS_FIRST)
		{
			p_mb->reg_v[0] = p_mode;
			if (!(p_mb->reg_v[0] & p_mode_en))
				p_mb->reg_v[0] = MODE_SUMMER;
		}
		else
		{
			p_mode = p_mb->reg_v[0];
			p_mb->reg_v[0] = 0;
		}

		//Регистр B02
		if (!(p_mb->reg_v[1] & (p_speed_en & SPEED_AUTO)) && (p_mb->reg_v[1] > (p_speed_en & 0xff)))
			p_mb->reg_v[1] = MODE_AUTO;
		else if (s_state & SYS_FIRST)
		{
			p_mb->reg_v[1] = p_speed;
			if (!(p_mb->reg_v[1] & (p_speed_en & SPEED_AUTO)) && (p_mb->reg_v[1] > (p_speed_en & 0xff)))
				p_mb->reg_v[1] = MODE_AUTO;
		}
		else
		{
			p_speed = p_mb->reg_v[1];
			p_mb->reg_v[1] = 0;
		}

		//Регистр B03
		if (!(p_mb->reg_v[2] & p_inout_en))
			p_mb->reg_v[2] = SENS_LOC;
		else
		{
			p_inout = p_mb->reg_v[2];
			p_mb->reg_v[2] = 0;
		}

		//Регистры B04 - B0A
		p_mb->reg_v[3] = s_er;
		p_mb->reg_v[4] = time.dd;	//День
		p_mb->reg_v[5] = time.mn;	//Месяц
		p_mb->reg_v[6] = time.yr;	//Год

		if (time.cn == 0)
			p_mb->reg_v[6] += 2000;
		else
			p_mb->reg_v[6] += 1900;

		p_mb->reg_v[7] = time.wd;	//День недели
		p_mb->reg_v[8] = time.hh;	//Часы
		p_mb->reg_v[9] = time.mm;	//Минуты

		//Регистр B0B
		if (s_state & SYS_FIRST)
		{
			p_mb->reg_v[10] = p_set_t/10;
			p_mb->cnt = 11;
		}
		if (p_mb->reg_v[10] < 5)
		{
			p_mb->reg_v[10] = 5;
			p_mb->cnt = 11;
		}
		else if (p_mb->reg_v[10] > 50)
		{
			p_mb->reg_v[10] = 50;
			p_mb->cnt = 11;
		}
		p_set_t = p_mb->reg_v[10] * 10;

		//Регистр B0C
		if (p_mb->reg_v[11] < 1)
		{
			p_mb->reg_v[11] = 1;
			p_mb->cnt = 12;
		}
		else if (p_mb->reg_v[11] > 100)
		{
			p_mb->reg_v[11] = 100;
			p_mb->cnt = 12;
		}
		p_set_h = p_mb->reg_v[11];

		//Регистр B0D
		ainp = p_mb->reg_v[12];

		s_state &= ~SYS_FIRST;

		p_line = P_LINE_BW;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

void p_line_bw()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->func = MB_FW;
		reg_mwrite(p_mb);
	}
	else
	{
		p_line = P_LINE_CW;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

//Параметры группы C
void p_line_cr()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->func = MB_FR;
		p_mb->reg_n = 0x0c00;
		p_mb->cnt = 15;
		reg_read(p_mb);
	}
	else
	{
	
		p_line = P_LINE_CW;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

void p_line_cw()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->reg_v[0] = 0;
		p_mb->reg_v[1] = ain[0];
		p_mb->reg_v[2] = 0;
		p_mb->reg_v[3] = 0;
		p_mb->reg_v[4] = 0;
		p_mb->reg_v[5] = 0;
		p_mb->reg_v[6] = 0;
		p_mb->reg_v[7] = dout;
		p_mb->reg_v[8] = heat_p;
		p_mb->reg_v[9] = 0;
		p_mb->reg_v[10] = 0;
		p_mb->reg_v[11] = 0;
		p_mb->reg_v[12] = 0;
		p_mb->reg_v[13] = 0;
		p_mb->reg_v[14] = 0;

		p_mb->func = MB_FW;
		p_mb->reg_n = 0x0c00;
		p_mb->cnt = 15;
		reg_mwrite(p_mb);
	}
	else
	{
		p_line = P_LINE_STAT;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

//Статус панели

void p_line_stat()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->func = MB_FR;
		p_mb->reg_n = 0x0b00;
		p_mb->cnt = 1;
		reg_read(p_mb);
	}
	else
	{
		p_status = p_mb->reg_v[0];

		if (p_status & (1<<P_STAT_MENU))
			p_mb->t_accept_set = T_MB_ACCEPT_M;
		else
			p_mb->t_accept_set = T_MB_ACCEPT;

		if (p_status & (1<<P_STAT_MENU_READ))
		{
			p_menu = p_status>>12;
			p_line = P_LINE_MR;
			p_mb->accept = 1;
		}
		else if (p_status & (1<<P_STAT_MENU_WRITE))
		{
			p_menu = p_status>>12;
			p_line = P_LINE_MW;
			p_mb->accept = 1;
		}
		else if ((p_status & (1<<P_STAT_TIMER)) && ((((p_status & (1<<P_STAT_RUN)) > 0) && ((s_state & SYS_START) == 0)) || (((p_status & (1<<P_STAT_RUN)) == 0) && ((s_state & SYS_START) > 0))))
		{
			p_line = P_LINE_E0;
			p_mb->accept = 1;
		}
		else if ((((p_status & (1<<P_STAT_LED0)) > 0) && ((s_state & SYS_ALARM) == 0)) || (((p_status & (1<<P_STAT_LED0)) == 0) && ((s_state & SYS_ALARM) > 0)))
		{
			p_line = P_LINE_E1;
			p_mb->accept = 1;
		}
		else if ((((p_status & (1<<P_STAT_LED1)) > 0) && ((s_state & SYS_WORK) == 0)) || (((p_status & (1<<P_STAT_LED1)) == 0) && ((s_state & SYS_WORK) > 0)))
		{
			p_line = P_LINE_E2;
			p_mb->accept = 1;
		}
		else if ((((p_status & (1<<P_STAT_BLOCK)) > 0) && ((s_state & SYS_BLOCK) == 0)) || (((p_status & (1<<P_STAT_BLOCK)) == 0) && ((s_state & SYS_BLOCK) > 0)))
		{
			p_line = P_LINE_E3;
			p_mb->accept = 1;
		}
		else
		{
			p_line = P_LINE_BR;
		}

		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
	}
}

void p_line_mr()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->cnt = 16;
		switch (p_menu)
		{
		case MENU_PARAM:
			for (i = 0; i < 16; i++)
				p_mb->reg_v[i] = eeprom_read_word(&p_menu_1[i]);
			p_mb->cnt = 16;
		break;
		case MENU_SET:
			for (i = 0; i < 16; i++)
				p_mb->reg_v[i] = eeprom_read_word(&p_menu_2[i]);
			p_mb->cnt = 16;
		break;
		case MENU_MON:
			for (i = 0; i < 12; i++)
				p_mb->reg_v[i] = eeprom_read_word(&p_day_0[i]);
			p_mb->cnt = 12;
		break;
		case MENU_TUE:
			for (i = 0; i < 12; i++)
				p_mb->reg_v[i] = eeprom_read_word(&p_day_1[i]);
			p_mb->cnt = 12;
		break;
		case MENU_WED:
			for (i = 0; i < 12; i++)
				p_mb->reg_v[i] = eeprom_read_word(&p_day_2[i]);
			p_mb->cnt = 12;
		break;
		case MENU_THU:
			for (i = 0; i < 12; i++)
				p_mb->reg_v[i] = eeprom_read_word(&p_day_3[i]);
			p_mb->cnt = 12;
		break;
		case MENU_FRI:
			for (i = 0; i < 12; i++)
				p_mb->reg_v[i] = eeprom_read_word(&p_day_4[i]);
			p_mb->cnt = 12;
		break;
		case MENU_SAT:
			for (i = 0; i < 12; i++)
				p_mb->reg_v[i] = eeprom_read_word(&p_day_5[i]);
			p_mb->cnt = 12;
		break;
		case MENU_SUN:
			for (i = 0; i < 12; i++)
				p_mb->reg_v[i] = eeprom_read_word(&p_day_6[i]);
			p_mb->cnt = 12;
		break;
		case MENU_TIME:
			p_mb->reg_v[0] = time.dd;
			p_mb->reg_v[1] = time.mn;
			p_mb->reg_v[2] = time.yr;
			if (time.cn == 1)
				p_mb->reg_v[2] += 1900;
			else
				p_mb->reg_v[2] += 2000;
			p_mb->reg_v[3] = time.wd;
			p_mb->reg_v[4] = time.hh;
			p_mb->reg_v[5] = time.mm;
			p_mb->cnt = 6;
		break;
		default:
			for (i = 0; i < 16; i++)
				p_mb->reg_v[i] = 0;
		}

		p_mb->func = MB_FW;
		p_mb->reg_n = 0x0d00;
		reg_mwrite(p_mb);
	}
	else
	{
		p_line = P_LINE_MC;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

//unsigned char eeprom_var[32] EEMEM;

void p_line_mw()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->func = MB_FR;
		p_mb->reg_n = 0x0d00;
		p_mb->cnt = 16;
		reg_read(p_mb);
	}
	else
	{
		i = 0;
		j = 0;
		if (p_menu < 3)
			tmpc = 16;
		else
			tmpc = 12;
		while (j < 64 && i < tmpc)
		{
			switch (p_menu)
			{
			case MENU_PARAM:
				eeprom_write_word(&p_menu_1[i], p_mb->reg_v[i]);
				if (eeprom_read_word(&p_menu_1[i]) == p_mb->reg_v[i])
					i++;
			break;
			case MENU_SET:
				eeprom_write_word(&p_menu_2[i], p_mb->reg_v[i]);
				if (eeprom_read_word(&p_menu_2[i]) == p_mb->reg_v[i])
					i++;
			break;
			case MENU_MON:
				eeprom_write_word(&p_day_0[i], p_mb->reg_v[i]);
				if (eeprom_read_word(&p_day_0[i]) == p_mb->reg_v[i])
					i++;
			break;
			case MENU_TUE:
				eeprom_write_word(&p_day_1[i], p_mb->reg_v[i]);
				if (eeprom_read_word(&p_day_1[i]) == p_mb->reg_v[i])
					i++;
			break;
			case MENU_WED:
				eeprom_write_word(&p_day_2[i], p_mb->reg_v[i]);
				if (eeprom_read_word(&p_day_2[i]) == p_mb->reg_v[i])
					i++;
			break;
			case MENU_THU:
				eeprom_write_word(&p_day_3[i], p_mb->reg_v[i]);
				if (eeprom_read_word(&p_day_3[i]) == p_mb->reg_v[i])
					i++;
			break;
			case MENU_FRI:
				eeprom_write_word(&p_day_4[i], p_mb->reg_v[i]);
				if (eeprom_read_word(&p_day_4[i]) == p_mb->reg_v[i])
					i++;
			break;
			case MENU_SAT:
				eeprom_write_word(&p_day_5[i], p_mb->reg_v[i]);
				if (eeprom_read_word(&p_day_5[i]) == p_mb->reg_v[i])
					i++;
			break;
			case MENU_SUN:
				eeprom_write_word(&p_day_6[i], p_mb->reg_v[i]);
				if (eeprom_read_word(&p_day_6[i]) == p_mb->reg_v[i])
					i++;
			break;
			}

			j++;
		}
		if (p_menu == MENU_TIME)
		{
			time.state = 1;
			time.ss = 0;
			time.dd = p_mb->reg_v[0];
			if (time.dd > 31)
				time.dd = 31;
			time.mn = p_mb->reg_v[1];
			if (time.mn > 12)
				time.mn = 12;

			if (p_mb->reg_v[2] > 2099)
				p_mb->reg_v[2] = 2099;
			else if (p_mb->reg_v[2] < 1900)
				p_mb->reg_v[2] = 1900;

			if (p_mb->reg_v[2] > 1999)
			{
				time.yr = ((int)p_mb->reg_v[2] - (int)2000);
				time.cn = 0;
			}
			else
			{
				time.yr = ((int)p_mb->reg_v[2] - (int)1900);
				time.cn = 1;
			}

			time.wd = p_mb->reg_v[3];
			if (time.wd > 6)
				time.wd = 6;
			time.hh = p_mb->reg_v[4];
			if (time.hh > 23)
				time.hh = 23;
			time.mm = p_mb->reg_v[5];
			if (time.mm > 59)
				time.mm = 59;
		}

		p_line = P_LINE_MC;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

void p_line_mc()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->reg_v[0] = 1;
		p_mb->func = MB_FW;
		p_mb->reg_n = (0x0e00|P_STAT_MENU_READY);
		coil_write(p_mb);
	}
	else
	{
		p_line = P_LINE_BR;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
	}
}

void p_line_e0()
{
	if (!(p_mb->state & MB_WAIT))
	{
		if (s_state & SYS_START)
			p_mb->reg_v[0] = 1;
		else
			p_mb->reg_v[0] = 0;
		p_mb->func = MB_FW;
		p_mb->reg_n = (0x0e00|P_STAT_RUN);
		coil_write(p_mb);
	}
	else
	{
		p_line = P_LINE_STAT;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

void p_line_e1()
{
	if (!(p_mb->state & MB_WAIT))
	{
		if (s_state & SYS_ALARM)
			p_mb->reg_v[0] = 1;
		else
			p_mb->reg_v[0] = 0;
		p_mb->func = MB_FW;
		p_mb->reg_n = (0x0e00|P_STAT_LED0);
		coil_write(p_mb);
	}
	else
	{
		p_line = P_LINE_STAT;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

void p_line_e2()
{
	if (!(p_mb->state & MB_WAIT))
	{
		if (s_state & SYS_WORK)
			p_mb->reg_v[0] = 1;
		else
			p_mb->reg_v[0] = 0;
		p_mb->func = MB_FW;
		p_mb->reg_n = (0x0e00|P_STAT_LED1);
		coil_write(p_mb);
	}
	else
	{
		p_line = P_LINE_STAT;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

void p_line_e3()
{
	if (!(p_mb->state & MB_WAIT))
	{
		if (s_state & SYS_BLOCK)
			p_mb->reg_v[0] = 1;
		else
			p_mb->reg_v[0] = 0;
		p_mb->func = MB_FW;
		p_mb->reg_n = (0x0e00|P_STAT_BLOCK);
		coil_write(p_mb);
	}
	else
	{
		p_line = P_LINE_STAT;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}

void p_line_e4()
{
	if (!(p_mb->state & MB_WAIT))
	{
		p_mb->reg_v[0] = 1;
		p_mb->func = MB_FW;
		p_mb->reg_n = (0x0e00|P_STAT_TIMER);
		coil_write(p_mb);
	}
	else
	{
		p_line = P_LINE_STAT;
		p_mb->state &= ~(MB_PROC|MB_WAIT|MB_REP);
		p_mb->accept = 1;
	}
}
