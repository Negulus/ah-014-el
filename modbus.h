#ifndef __MODBUS_H__
#define __MODBUS_H__ 1

//#define F_CPU 8000000
#define USART_BAUDRATE 9600 
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) 

#define MB_RX		(1<<0)	//����
#define MB_TX		(1<<1)	//��������
#define MB_WAIT		(1<<2)	//�������� ������
#define MB_REP		(1<<3)	//��������� ������� �������
#define MB_PROC		(1<<4)	//��������� ������
#define MB_WRITE	(1<<5)	//������� ������

#define MB_PR_NONE	0x00
#define MB_PR_EVEN	0x02
#define MB_PR_ODD	0x03

#define MB_SB_1		0x00
#define MB_SB_2		0x01

#define MB_ER_ADR	(1<<0)	//������ �����
#define MB_ER_OF	(1<<1)	//������������ ��� �����
#define MB_ER_PR	(1<<2)	//������ ��������
#define MB_ER_TIME	(1<<3)	//���������� ����� ��������

#define MB_MASTER	0x01
#define MB_SLAVE	0x02

#define MB_ADR		0x01

#define MB_TRY		2

#define MB_FR		0x01
#define MB_FW		0x02
#define MB_FC		0x03

#define ER_FUNC		0x01
#define ER_ADR		0x02
#define ER_DATA		0x03
#define	ER_SLAVE	0x04
#define	ER_BUSY		0x06

#define	MB_TIMEOUT		500
#define	MB_END_TIMEOUT	5

typedef struct MB_DATA
{
	unsigned char	state;		//������ ModBus
	unsigned char	error;		//������ ModBus
	unsigned char	type;		//��� ���������� (������, �����)
	unsigned int	reg_n;		//��������� �������
	unsigned int	reg_v[16];	//����� ���������
	unsigned char	cnt;		//���������� ��������� � ������
	unsigned char	num;		//���������� ���� � ���������
	unsigned char	rx[48];		//����� �����
	unsigned char	tx[48];		//����� ��������
	unsigned char	ctu;		//������� ����
	unsigned char	try;		//���������� �������
	unsigned char	crc[2];		//����� ��� �������� ����������� �����
	unsigned int	crc16;		//����� ��� ������� ����������� �����
	unsigned char	accept;
	unsigned char	func;
	unsigned char	tmp;

	volatile uint8_t	*uscra;
	volatile uint8_t	*uscrb;
	volatile uint8_t	*uscrc;
	volatile uint8_t	*ubrrl;
	volatile uint8_t	*ubrrh;
	volatile uint8_t	*udr;
	volatile uint8_t	*ddr;
	volatile uint8_t	*port;
	unsigned char		bit;

	unsigned int	*reg_tmp;

	//������� ������ ModBus
	unsigned int  t;
	unsigned int  t_set;
	unsigned int  t_accept;
	unsigned int  t_accept_set;
	unsigned int  t_next;
	unsigned int  t_next_set;
	unsigned int  t_check;
	unsigned int  t_stoptx;
	unsigned int  t_stoptx_set;
} mb_data;

mb_data	mb0;
mb_data	mb1;

void	mb_init(mb_data *mb, unsigned char mbt, volatile uint8_t *uscra, volatile uint8_t *uscrb, volatile uint8_t *uscrc, volatile uint8_t *ubrrl, volatile uint8_t *ubrrh, volatile uint8_t *udr, volatile uint8_t *ddr, volatile uint8_t *port, unsigned char bit, unsigned char sb, unsigned char pr);
void	mb_start_tx(mb_data *mb);
void	mb_start_rx(mb_data *mb);
void	mb_stop_rx(mb_data *mb);
void	mb_stop_tx(mb_data *mb);
void	mb_rx_new(mb_data *mb);
void	mb_tx_next(mb_data *mb);
void	reg_write(mb_data *mb);
void	reg_mwrite(mb_data *mb);
void	reg_read(mb_data *mb);
void	coil_write(mb_data *mb);
void	crc_calc(mb_data *mb, unsigned char *m_xx);

//������� ��������
void	timer_mb(mb_data *mb);
void	timer_mb_set(mb_data *mb, unsigned int set);
void	timer_mb_stoptx_set(mb_data *mb, unsigned int set);
void	timer_mb_accept(mb_data *mb);
void	timer_mb_check(mb_data *mb);
void	timer_mb_next(mb_data *mb);
void	timer_mb_next_set(mb_data *mb, unsigned int set);

//������� ��� ��������� �������� ������
void	mb_proc_start(mb_data *mb);
void	 ans_01(mb_data *mb, unsigned int *reg_tmp);
void	 ans_03(mb_data *mb, unsigned int *reg_tmp);
void	 ans_error(mb_data *mb, unsigned char er);

#endif
