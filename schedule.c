#include "main.h"
#include "panel.h"
#include "twi.h"
#include "system.h"

void shed_init()
{
	for (i = 0; i < 12; i++)
		shed_day[i] = 0;
}

void shed_upd()
{
	switch (time.wd)
	{
	case 0:
		for (i = 0; i < 12; i++)
			shed_day[i] = eeprom_read_word(&p_day_0[i]);
	break;
	case 1:
		for (i = 0; i < 12; i++)
			shed_day[i] = eeprom_read_word(&p_day_1[i]);
	break;
	case 2:
		for (i = 0; i < 12; i++)
			shed_day[i] = eeprom_read_word(&p_day_2[i]);
	break;
	case 3:
		for (i = 0; i < 12; i++)
			shed_day[i] = eeprom_read_word(&p_day_3[i]);
	break;
	case 4:
		for (i = 0; i < 12; i++)
			shed_day[i] = eeprom_read_word(&p_day_4[i]);
	break;
	case 5:
		for (i = 0; i < 12; i++)
			shed_day[i] = eeprom_read_word(&p_day_5[i]);
	break;
	case 6:
		for (i = 0; i < 12; i++)
			shed_day[i] = eeprom_read_word(&p_day_6[i]);
	break;
	}
}

void shedule()
{
	if (!(p_status & (1<<P_STAT_TIMER)))
		return;

	if ((time.hh > shed_day[0]) && (time.hh < shed_day[2]))
	{
		shed_on(0);
		return;
	}
	else if ((time.hh == shed_day[0]) && (time.hh == shed_day[2]))
	{
		if ((time.mm >= shed_day[1]) && (time.mm < shed_day[3]))
		{
			shed_on(0);
			return;
		}
	}
	else if (time.hh == shed_day[0])
	{
		if (time.mm >= shed_day[1])
		{
			shed_on(0);
			return;
		}
	}
	else if (time.hh == shed_day[2])
	{
		if (time.mm < shed_day[3])
		{
			shed_on(0);
			return;
		}
	}

	else if ((time.hh > shed_day[6]) && (time.hh < shed_day[8]))
	{
		shed_on(1);
		return;
	}
	else if ((time.hh == shed_day[6]) && (time.hh == shed_day[8]))
	{
		if ((time.mm >= shed_day[7]) && (time.mm < shed_day[9]))
		{
			shed_on(1);
			return;
		}
	}
	else if (time.hh == shed_day[6])
	{
		if (time.mm >= shed_day[7])
		{
			shed_on(1);
			return;
		}
	}
	else if (time.hh == shed_day[8])
	{
		if (time.mm < shed_day[9])
		{
			shed_on(1);
			return;
		}
	}
	shed_off();
}

void shed_on(unsigned char num)
{
	if (p_status & (1<<P_STAT_TIMER_BLOCK))
	{
		shed_off();
		return;
	}

	if (!(s_state & SYS_START))
	{
		num *= 6;
		s_state |= SYS_FIRST;
		p_set_t = shed_day[4+num];
		p_speed = shed_day[5+num];
	}

	s_state |= SYS_START;
	return;
}

void shed_off()
{
	s_state &= ~SYS_START;
	return;
}
