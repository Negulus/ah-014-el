#include <avr/interrupt.h>
#include "twi.h"
#include "main.h"

void twi_init()
{
	twi.state = 0;
	twi.accept = 0;
	twi.func = 0;
	twi.cnt = 0;
	twi.ctu = 0;
	twi.adr = 0;
	twi.reg = 0;
	twi.line = TWI_LINE_C1_W;

	t_twi = 0;
	t_twi_set = 2000;

	for (int i = 0; i < 8; i++)
		twi.data[i] = 0;

	TWSR = 0;
	TWBR = ((F_CPU/SCL_CLOCK)-16)/2;
}

void twi_read(unsigned char adr, unsigned char reg, unsigned char cnt)
{
	twi.state = 1;
	twi.adr = adr;
	twi.reg = reg;
	twi.ctu = 0;
	twi.cnt = cnt;
	twi.func = 1;
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
	twi.accept = 0;
	TWCR |= (1<<TWSTA);
}

void twi_write(unsigned char adr, unsigned char reg, unsigned char cnt)
{
	twi.state = 1;
	twi.adr = adr;
	twi.reg = reg;
	twi.ctu = 0;
	twi.cnt = cnt;
	twi.func = 0;
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
	twi.accept = 0;
	TWCR |= (1<<TWSTA);
}

ISR (TWI_vect)
{
	switch ((TWSR&0b11111000))
	{
	//START condition has been transmitted
	case 0x08:
		TWDR = twi.adr;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
		return;
	break;

	//repeated START condition has been transmitted
	case 0x10:
		TWDR = twi.adr + twi.func;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
		return;
	break;

	//SLA+W has been transmitted; ACK has been received
	case 0x18:
		TWDR = twi.reg;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
		return;
	break;

	//Data byte has been transmitted; ACK has been received
	case 0x28:
		if (twi.func == TWI_RD)
		{
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTA);
			return;
		}
		else
		{
			if (twi.ctu < twi.cnt)
			{
				TWDR = twi.data[twi.ctu];
				twi.ctu++;
				TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
				return;
			}
			else
			{
				t_twi_set = TWI_TIME;
//				twi.line++;
				twi.state = 0;
				TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTO);
				return;
			}
		}
	break;

	//SLA+R has been transmitted; ACK has been received
	case 0x40:
		if (twi.cnt > 1)
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		else
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
		return;
	break;

	//Data byte has been received; ACK has been returned
	case 0x50:
		twi.data[twi.ctu] = TWDR;
		twi.ctu++;
		if ((twi.ctu >= twi.cnt) || (twi.ctu > 7))
		{
			time_proc();
			t_twi_set = TWI_TIME;
//			twi.line++;
			twi.state = 0;
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTO);
			return;
		}
		else
		{
			if (twi.cnt > 1)
				TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
			else
				TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
			return;
		}
	break;

	//Data byte has been received; NOT ACK has been returned
	case 0x58:
		twi.data[twi.ctu] = TWDR;
		time_proc();
		t_twi_set = TWI_TIME;
//		twi.line++;
		twi.state = 0;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTO);
		return;
	break;

	//Data byte has been transmitted; NOT ACK has been received
	case 0x30:

	//SLA+W has been transmitted; NOT ACK has been received
	case 0x20:

	//Arbitration lost in SLA+R or NOT ACK bit
	case 0x38:

	//SLA+R has been transmitted; NOT ACK has been received
	case 0x48:

	default:
		t_twi_set = TWI_TIME;
		twi.state = 0;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTO);
		return;
	break;
	}

	return;
}

void time_proc()
{
	if (time.state == 1)
		twi.line = TWI_LINE_SS_W;

	switch (twi.line)
	{
	case TWI_LINE_SS_R:
		time.ss = BCD2DEC(twi.data[0] & 0b01111111);
		twi.line = TWI_LINE_MM_R;
	break;
	case TWI_LINE_MM_R:
		time.mm = BCD2DEC(twi.data[0] & 0b01111111);
		twi.line = TWI_LINE_HH_R;
	break;
	case TWI_LINE_HH_R:
		time.hh = BCD2DEC(twi.data[0] & 0b00111111);
		twi.line = TWI_LINE_DD_R;
	break;
	case TWI_LINE_DD_R:
		time.dd = BCD2DEC(twi.data[0] & 0b00111111);
		twi.line = TWI_LINE_WD_R;
	break;
	case TWI_LINE_WD_R:
		time.wd = twi.data[0] & 0b00000111;
		twi.line = TWI_LINE_MN_R;
	break;
	case TWI_LINE_MN_R:
		time.mn = BCD2DEC(twi.data[0] & 0b00011111);
		if (twi.data[0] & 0b10000000)
			time.cn = 1;
		else
			time.cn = 0;	
		twi.line = TWI_LINE_YR_R;
	break;
	case TWI_LINE_YR_R:
		time.yr = BCD2DEC(twi.data[0]);
		twi.line = TWI_LINE_SS_R;
	break;

	}
}

void twi_queue()
{
	if (twi.accept == 1)
	{
		if (time.state == 1)
		{
			twi.line = TWI_LINE_SS_W;
			time.state = 0;
		}

		switch (twi.line)
		{
		//Настройка часов
		case TWI_LINE_C1_W:
			time.state = 0;
			twi.data[0] = 0b00000000;
			twi_write(TWI_ADR, 0x00, 1);
			twi.line = TWI_LINE_C2_W;
		break;
		case TWI_LINE_C2_W:
			twi.data[0] = 0b00000000;
			twi_write(TWI_ADR, 0x01, 1);
			twi.line = TWI_LINE_SS_R;
		break;

		//Чтение данных часов
		case TWI_LINE_SS_R:
			twi_read(TWI_ADR, 0x02, 1);
		break;
		case TWI_LINE_MM_R:
			twi_read(TWI_ADR, 0x03, 1);
		break;
		case TWI_LINE_HH_R:
			twi_read(TWI_ADR, 0x04, 1);
		break;
		case TWI_LINE_DD_R:
			twi_read(TWI_ADR, 0x05, 1);
		break;
		case TWI_LINE_WD_R:
			twi_read(TWI_ADR, 0x06, 1);
		break;
		case TWI_LINE_MN_R:
			twi_read(TWI_ADR, 0x07, 1);
		break;
		case TWI_LINE_YR_R:
			twi_read(TWI_ADR, 0x08, 1);
		break;
		case TWI_LINE_SS_W:
			twi.data[0] = DEC2BCD(time.ss);
			twi_write(TWI_ADR, 0x02, 1);
			twi.line = TWI_LINE_MM_W;
		break;
		case TWI_LINE_MM_W:
			twi.data[0] = DEC2BCD(time.mm);
			twi_write(TWI_ADR, 0x03, 1);
			twi.line = TWI_LINE_HH_W;
		break;
		case TWI_LINE_HH_W:
			twi.data[0] = DEC2BCD(time.hh);
			twi_write(TWI_ADR, 0x04, 1);
			twi.line = TWI_LINE_DD_W;
		break;
		case TWI_LINE_DD_W:
			twi.data[0] = DEC2BCD(time.dd);
			twi_write(TWI_ADR, 0x05, 1);
			twi.line = TWI_LINE_WD_W;
		break;
		case TWI_LINE_WD_W:
			twi.data[0] = time.wd;
			twi_write(TWI_ADR, 0x06, 1);
			twi.line = TWI_LINE_MN_W;
		break;
		case TWI_LINE_MN_W:
			twi.data[0] = DEC2BCD(time.mn);
			if (time.cn == 1)
				twi.data[0] |= 0b10000000;
			twi_write(TWI_ADR, 0x07, 1);
			twi.line = TWI_LINE_YR_W;
		break;
		case TWI_LINE_YR_W:
			twi.data[0] = DEC2BCD(time.yr);
			twi_write(TWI_ADR, 0x08, 1);
			twi.line = TWI_LINE_SS_R;
		break;

		default:
			twi.line = TWI_LINE_SS_R;
		}
	}
}

char BCD2DEC(char in)
{
	return (((in >> 4) & 0x0f) * 10) + (in & 0x0f);
}

char DEC2BCD(char in)
{
	return ((in/10)<<4) + (in - ((in/10)*10));
}

