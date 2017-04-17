#include "modbus.h"

//������
#define	MODE_AUTO		(1<<0)
#define MODE_SUMMER		(1<<1)
#define MODE_WINTER		(1<<2)

//����� (�������)
#define SENS_OUT		(1<<0)
#define SENS_AIR		(1<<1)
#define SENS_WATER		(1<<2)
#define SENS_EX			(1<<3)
#define SENS_ROOM		(1<<4)
#define SENS_LOC		(1<<5)
#define SENS_OUT_H		(1<<6)
#define SENS_AIR_H		(1<<7)
#define SENS_ROOM_H		(1<<8)

//������
#define OUT_HEAT		(1<<9)
#define OUT_COOL		(1<<10)
#define OUT_VENT1		(1<<11)
#define OUT_VENT2		(1<<12)
#define OUT_REC			(1<<13)
#define OUT_HUM			(1<<14)
#define OUT_DHUM		(1<<15)

#define	MENU_MAIN	0
#define	MENU_PARAM	1
#define	MENU_SET	2
#define	MENU_CONF	3
#define	MENU_TIMER	4
#define	MENU_TIME	5
#define	MENU_MON	6
#define	MENU_TUE	7
#define	MENU_WED	8
#define	MENU_THU	9
#define	MENU_FRI	10
#define	MENU_SAT	11
#define	MENU_SUN	12

#define SPEED_AUTO	(1<<8)

#define	P_LINE_AR	1
#define	P_LINE_AW	2
#define	P_LINE_BR	3
#define	P_LINE_BW	4
#define	P_LINE_CR	5
#define	P_LINE_CW	6
#define	P_LINE_STAT	7
#define	P_LINE_MR	8
#define	P_LINE_MW	9
#define	P_LINE_MC	10
#define	P_LINE_E0	11
#define	P_LINE_E1	12
#define	P_LINE_E2	13
#define	P_LINE_E3	14
#define	P_LINE_E4	15

/*
#define	P_STAT_RUN			0	//������ �������
#define	P_STAT_LED0			1	//��������� �������
#define	P_STAT_LED1			2	//��������� ������
#define	P_STAT_BLOCK		3	//���������� ������
#define	P_STAT_TIMER		4	//������ �� �������
#define	P_STAT_MENU_READY	5	//���� ������
#define	P_STAT_MENU			6	//���� �������
#define	P_STAT_MENU_READ	7	//���� ������
#define	P_STAT_MENU_WRITE	8	//���� ������
#define	P_STAT_SYNC			9	//�������������
#define	P_STAT_SEL			10	//��������� ������
#define	P_STAT_CLEAR		11	//������� ������
*/
/*
#define	P_STAT_RUN			0	//������ �������
#define	P_STAT_LED0			1	//��������� �������
#define	P_STAT_LED1			2	//��������� ������
#define	P_STAT_BLOCK		3	//���������� �������
#define	P_STAT_TIMER		4	//������ �� �������
#define	P_STAT_MENU_READY	5	//���� ������ (������ ��� ������)
#define	P_STAT_MENU			5	//���� ������� (������ ��� ������)
#define	P_STAT_MENU_READ	6	//���� ������ (������ ��� ������)
#define	P_STAT_MENU_WRITE	7	//���� ������ (������ ��� ������)
#define	P_STAT_SYNC			8	//������������� (������ ��� ������)
#define	P_STAT_SEL			9	//��������� ������ (������ ��� ������)
#define	P_STAT_CLEAR		10	//������� ������ (������ ��� ������)
#define	P_STAT_ALARM_R		11	//����� ������ (������ ��� ������)
*/

#define	P_STAT_RUN			0	//������ �������
#define	P_STAT_LED0			1	//��������� �������
#define	P_STAT_LED1			2	//��������� ������
#define	P_STAT_BLOCK		3	//���������� �������
#define	P_STAT_TIMER		4	//������ �� �������
#define	P_STAT_TIMER_BLOCK	5	//���������� ������ �� �������
#define	P_STAT_MENU_READY	6	//���� ������ (������ ��� ������)
#define	P_STAT_MENU			6	//���� ������� (������ ��� ������)
#define	P_STAT_MENU_READ	7	//���� ������ (������ ��� ������)
#define	P_STAT_MENU_WRITE	8	//���� ������ (������ ��� ������)
#define	P_STAT_SYNC			9	//������������� (������ ��� ������)
#define	P_STAT_ALARM_R		10	//����� ������ (������ ��� ������)

#define	P_STAT_RUN_E		(1<<0)	//������ ������� ��������
#define	P_STAT_RUN_D		(1<<1)	//������ ������� ���������
#define	P_STAT_TIMER_E		(1<<2)	//������ �� ������� ��������
#define	P_STAT_TIMER_D		(1<<3)	//������ �� ������� ���������

//���������� ��� ������ � �������
unsigned char	p_line;
mb_data 		*p_mb;

//������� ������ � �������
void p_init(mb_data *mb);
void p_queue();
void p_line_ar();
void p_line_aw();
void p_line_br();
void p_line_bw();
void p_line_cr();
void p_line_cw();
void p_line_stat();
void p_line_mr();
void p_line_mw();
void p_line_mc();
void p_line_e0();
void p_line_e1();
void p_line_e2();
void p_line_e3();
void p_line_e4();

//���������� ������
unsigned int 	p_menu_line_en[3];	//�������� ������ ����
unsigned int 	p_status;			//������� ������
unsigned int 	p_mode_en;			//�������� ������
unsigned int 	p_mode;				//������� ������
unsigned int 	p_speed_en;			//�������� ��������
unsigned int 	p_speed;			//������� ��������
unsigned int 	p_inout_en;			//�������� ���������
unsigned int 	p_inout;			//������� ���������
int 			p_set_t;			//������� �����������
unsigned int 	p_set_h;			//������� ���������
unsigned char	p_menu;				//����� ��������� ����
