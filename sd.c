#include <avr/io.h>
#include "modbus.h"
#include "main.h"
#include "sd.h"
#include "system.h"
#include "panel.h"

void sd_init(mb_data *mb)
{
	sd_coils = 0;

	for (i = 0; i < 16; i++)
		sd_reg[i] = 0;

	sd_mb = mb;

	sd_state = 0;
}

void sd_update()
{
	sd_state |= (1<<0);

	sd_coils |= s_state;

	sd_reg[0] = ain[0];			//������ �� ����� ain0 � C
	sd_reg[1] = ain[1];			//������ �� ����� ain1 � C
	sd_reg[2] = ain[2];			//������ �� ����� ain2 � C
	sd_reg[3] = ain[3];			//������ �� ����� ain3 � C
	sd_reg[4] = ain[4];			//������ �� ����� ain4 � C
	sd_reg[5] = ain[5];			//������ �� ����� ain5 � %
	sd_reg[7] = ainp;			//������ � ������
	sd_reg[8] = aout0;			//����� aout0 � %
	sd_reg[9] = aout1;			//����� aout1 � %
	sd_reg[10] = aout2;			//����� aout2 � %
	sd_reg[11] = aout3;			//����� aout3 � %

	sd_state &= ~(1<<0);
}

void sd_ans()
{
	switch (sd_mb->rx[1])
	{
	//Read coils
	case 0x01:
		if (sd_mb->rx[5] == 0 && sd_mb->rx[4] == 0)
			ans_error(sd_mb, ER_DATA);
		else if ((sd_mb->rx[2] > 0) || (sd_mb->rx[4] > 0) || ((sd_mb->rx[3] + sd_mb->rx[5]) > 16))
			ans_error(sd_mb, ER_ADR);
		else
		{
			ans_01(sd_mb, &sd_coils);
		}
	break;

	//Read holding registers
	case 0x03:
		if (sd_mb->rx[5] == 0 && sd_mb->rx[4] == 0)
			ans_error(sd_mb, ER_DATA);
		else if ((sd_mb->rx[2] > 0) || (sd_mb->rx[4] > 0) || ((sd_mb->rx[3] + sd_mb->rx[5]) > 16))
			ans_error(sd_mb, ER_ADR);
		else
		{
			ans_03(sd_mb, &sd_reg[0]);
		}
	break;

	//Function error
	default:
		ans_error(sd_mb, ER_FUNC);
	break;
	}
}
