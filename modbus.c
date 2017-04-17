#include <avr/io.h>
#include <avr/interrupt.h>
#include "modbus.h"
#include "main.h"
#include "sd.h"
#include "panel.h"

//unsigned char ctu;

//Инициализация
void mb_init(mb_data *mb, unsigned char mbt, volatile uint8_t *uscra, volatile uint8_t *uscrb, volatile uint8_t *uscrc, volatile uint8_t *ubrrl, volatile uint8_t *ubrrh, volatile uint8_t *udr, volatile uint8_t *ddr, volatile uint8_t *port, unsigned char bit, unsigned char sb, unsigned char pr)
{
	mb->uscra = uscra;
	mb->uscrb = uscrb;
	mb->uscrc = uscrc;
	mb->ubrrl = ubrrl;
	mb->ubrrh = ubrrh;
	mb->udr = udr;
	mb->ddr = ddr;
	mb->port = port;
	mb->bit = bit;

	mb->state = 0;
	mb->error = 0;
	mb->type = mbt;
	mb->cnt = 0;
	mb->num = 0;
	mb->ctu = 0;
	mb->try = 0;
	mb->crc[0] = 0;
	mb->crc[1] = 0;
	mb->crc16 = 0;
	mb->accept = 0;
	mb->func = 0;

	*mb->uscra = 0;
	*mb->uscrb = 0;
	*mb->ubrrl = BAUD_PRESCALE;
	*mb->ubrrh = (BAUD_PRESCALE >> 8);
	*mb->uscrc = (1<<UCSZ00)|(1<<UCSZ01)|(pr<<4)|(sb<<3);
	*mb->ddr |= (1<<mb->bit);
	*mb->port &= ~(1<<mb->bit);

	mb->t = 0;
	mb->t_set = 0;
	mb->t_accept = 0;
	mb->t_accept_set = T_MB_ACCEPT;
	mb->t_next = 0;
	mb->t_next_set = 0;
	mb->t_check = 0;
	mb->t_stoptx = 0;
	mb->t_stoptx_set = 0;

	timer_mb_set(mb, 0);

	if (mbt == MB_SLAVE)
		mb_start_rx(mb);
}

//Начало приёма
void mb_start_rx(mb_data *mb)
{
	if ((mb->state & MB_TX) != 0)
	{
		*mb->uscrb &= ~(1<<3);
		*mb->port &= ~(1<<0);
		mb->state &= ~MB_TX;
	}

	if (mb->type == MB_MASTER)
		timer_mb_set(mb, MB_TIMEOUT);

	*mb->port &= ~(1<<mb->bit);

	mb->error = 0;
	mb->state |= MB_RX;
	mb->state &= ~MB_PROC;

	mb->ctu = 0;
	
	*mb->uscrb |= (1<<4);
	*mb->uscrb |= (1<<7);
}

//Приём байта
ISR (USART0_RX_vect)
{
	mb_rx_new(&mb0);
}

ISR (USART1_RX_vect)
{
	mb_rx_new(&mb1);
}

void mb_rx_new(mb_data *mb)
{
	mb->tmp = *mb->udr;
	if ((mb->state & MB_RX) == 0)
		return;

	timer_mb_set(mb, MB_END_TIMEOUT);

	if (mb->error != 0)
		return;

	if (*mb->uscra & (1<<UPE0))
		mb->error |= MB_ER_PR;

	mb->rx[mb->ctu] = mb->tmp;
	
	if (mb->ctu == 0)
	{
		if (mb->rx[0] != 0x01)
		{
			mb->error |= MB_ER_ADR;
			return;
		}
	}

	mb->ctu++;

	if (mb->ctu >= 48)
		mb->error |= MB_ER_OF;

	return;
}

//Остановка приёма
void mb_stop_rx(mb_data *mb)
{
	if ((mb->state & MB_RX) == 0)
		return;

	*mb->uscrb &= ~(1<<7);
	*mb->uscrb &= ~(1<<4);
	
	if ((mb->error & MB_ER_ADR) > 0)
	{
		mb_start_rx(mb);
		return;
	}

	if (mb->ctu == 0)
	{
		mb->error |= MB_ER_TIME;
	}

	mb->state |= MB_PROC;
	mb->state &= ~MB_RX;
	timer_mb_next_set(mb, T_MB_NEXT);
	mb_proc_start(mb);
}

//Начало передачи
void mb_start_tx(mb_data *mb)
{
	if ((mb->state & MB_RX) != 0)
	{
		*mb->uscrb &= ~(1<<7);
		*mb->uscrb &= ~(1<<4);
		mb->state &= ~MB_RX;
	}

	mb->state |= MB_TX;
	mb->state &= ~MB_PROC;
	mb->error = 0;

	if (mb->try < MB_TRY)
		mb->try++;

	if (mb->t_next < mb->t_next_set)
	{
		return;
	}

	mb->t_check = 0;

	*mb->uscrb = (1<<3);

	crc_calc(mb, &mb->tx[0]);
	mb->tx[mb->num] = mb->crc[1];
	mb->tx[mb->num+1] = mb->crc[0];
	mb->num += 2;
	mb->ctu = 0;

	*mb->port |= (1<<mb->bit);
	*mb->uscrb |= (1<<5);
}

//Отправка следующего байта
ISR (USART0_UDRE_vect)
{
	mb_tx_next(&mb0);
}

ISR (USART1_UDRE_vect)
{
	mb_tx_next(&mb0);
}

void mb_tx_next(mb_data *mb)
{
	if ((mb->state & MB_TX) == 0)
		return;

	if (mb->ctu >= mb->num)
	{
		timer_mb_stoptx_set(mb, 5);
		*mb->uscrb &= ~(1<<5);
	}
	else
	{
		*mb->udr = mb->tx[mb->ctu];
		mb->ctu++;
	}
}

//Окончание передачи (после отправки последнего байта)
void mb_stop_tx(mb_data *mb)
{
	if ((mb->state & MB_TX) == 0)
		return;

	*mb->uscrb &= ~(1<<3);
	*mb->port &= ~(1<<mb->bit);
	
	if (mb->type == MB_MASTER)
		mb->state |= MB_WAIT;

	mb->state &= ~MB_TX;

	mb_start_rx(mb);
	return;
}

//Подсчёт контрольной суммы
void crc_calc(mb_data *mb, unsigned char *mb_xx)
{
	mb->crc16=0xFFFF;
	for (i = 0; i < mb->num; i++)
	{
		mb->crc16 ^= *mb_xx++;
		for (j=0; j<8; j++)
		{
			if (mb->crc16 & 0x0001) 
			{
				mb->crc16 >>= 1;
				mb->crc16 ^= 0xA001;
			}
			else
				mb->crc16 >>= 1;
		}
	}
	mb->crc[1] = mb->crc16&0xff;
	mb->crc[0] = (mb->crc16 >> 8);
	return;
}

//Отправка одного бита (05h)
void coil_write(mb_data *mb)
{
	mb->tx[0] = 0x01;
	mb->tx[1] = 0x05;
	mb->tx[2] = (mb->reg_n>>8);
	mb->tx[3] = mb->reg_n&0xff;

	if (mb->reg_v[0] > 0)
		mb->tx[4] = 0xff;
	else
		mb->tx[4] = 0x00;

	mb->tx[5] = 0x00;
	mb->num = 6;
	mb_start_tx(mb);
}

//Отправка одного регистра (06h)
void reg_write(mb_data *mb)
{
	mb->tx[0] = 0x01;
	mb->tx[1] = 0x06;
	mb->tx[2] = (mb->reg_n>>8);
	mb->tx[3] = mb->reg_n&0xff;
	mb->tx[4] = (mb->reg_v[0] >> 8);
	mb->tx[5] = mb->reg_v[0]&0xff;
	mb->num = 6;
	mb_start_tx(mb);
}

//Отправка нескольких регистров (10h)
void reg_mwrite(mb_data *mb)
{
	if ((mb->reg_n + mb->cnt) < mb->reg_n || mb->cnt == 0)
		return;

	mb->tx[0] = 0x01;
	mb->tx[1] = 0x10;
	mb->tx[2] = (mb->reg_n>>8);
	mb->tx[3] = mb->reg_n&0xff;
	mb->tx[4] = 0;
	mb->tx[5] = mb->cnt;
	mb->tx[6] = mb->cnt*2;

	for (i = 0; i < mb->cnt; i++)
	{
		mb->tx[7+(i*2)] = (mb->reg_v[i]>>8);
		mb->tx[8+(i*2)] = mb->reg_v[i]&0xff;
	}

	mb->num = 7+(mb->cnt*2);

	mb_start_tx(mb);
}

//Чтение нескольких регистров (03h)
void reg_read(mb_data *mb)
{
	mb->tx[0] = 0x01;
	mb->tx[1] = 0x03;
	mb->tx[2] = (mb->reg_n>>8);
	mb->tx[3] = mb->reg_n&0xff;
	mb->tx[4] = 0;
	mb->tx[5] = mb->cnt;

	mb->num = 6;

	mb_start_tx(mb);
}

void ans_01(mb_data *mb, unsigned int *reg_tmp)
{
	mb->tx[0] = MB_ADR;
	mb->tx[1] = 0x01;

	tmp = *reg_tmp;
	tmp <<= (8 - (mb->rx[3] + mb->rx[5]));
	tmp >>= (8 - mb->rx[5]);
	mb->tx[3] = tmp&0xff;
	mb->num = 4;

	if (mb->rx[5] <= 8)
		mb->tx[2] = 1;
	else
	{
		mb->num++;
		mb->tx[2] = 2;
		mb->tx[4] = mb->tx[3];
		mb->tx[3] = (tmp >> 8);
	}
}

void ans_03(mb_data *mb, unsigned int *reg_tmp)
{
	mb->tx[0] = MB_ADR;
	mb->tx[1] = 0x03;
	mb->tx[2] = mb->rx[5] * 2;
	mb->num = 3 + mb->tx[2];

	for (i = 0; i < mb->rx[5]; i++)
	{
		mb->tx[(i*2)+3] = (*(reg_tmp + (i + mb->rx[3]))>>8);
		mb->tx[(i*2)+4] = *(reg_tmp + (i + mb->rx[3]))&0xff;
	}
}

void ans_error(mb_data *mb, unsigned char er)
{
	mb->tx[0] = MB_ADR;
	mb->tx[1] = mb->rx[1] + 0x80;
	mb->tx[2] = er;
	mb->num = 3;
}

void mb_proc_start(mb_data *mb)
{
	if (mb->type == MB_MASTER)
	{
		if (mb->error != 0)
		{
			mb->state = 0;
			if (mb->try < MB_TRY)
			{
				mb->accept = 1;
			}
			return;
		}

		if (mb->rx[1] > 0x80)
		{
			mb->state = 0;
			if (mb->try < MB_TRY)
			{
				mb->accept = 1;
			}
			return;
		}

		if (mb->rx[1] == 0x03)
		{
			if (mb->rx[2] / 2 != mb->cnt)
			{
				mb->state = 0;
				if (mb->try < MB_TRY)
				{
					mb->accept = 1;
				}
				return;
			}
			for (i = 0; i < mb->cnt; i++)
			{
				mb->reg_v[i] = mb->rx[3+(i*2)];
				mb->reg_v[i] <<= 8;
				mb->reg_v[i] |= mb->rx[4+(i*2)];
			}
		}

		if (p_mb == mb)
		{
			mb->try = 0;
			p_queue();
		}
	}
	else if (mb->type == MB_SLAVE)
	{
		if (mb->error == 0)
		{
			if (mb->rx[1] < 7)
				mb->num = 6;
			else if (mb->rx[1] == 0x0F)
				mb->num = mb->rx[6] * 2 + mb->rx[7];
			else if (mb->rx[1] == 0x10)
				mb->num = mb->rx[6] + mb->rx[7];
			else
				mb->num = 3;

			if (sd_mb == mb)
			{
				if (sd_state == 0)
					sd_ans();
				else
				{
					sd_state |= (1<<1);
					return;
				}
			}
			else
				ans_error(mb, ER_SLAVE);

			mb_start_tx(mb);
		}
		else
		{
			if ((mb->error & MB_ER_OF) > 0 || (mb->error & MB_ER_PR) > 0)
			{
				ans_error(mb, ER_FUNC);
				mb_start_tx(mb);
			}
			else
			{
				mb_start_rx(mb);
			}
		}
	}
}
