#include "modbus.h"

//Режимы
#define	MODE_AUTO		(1<<0)
#define MODE_SUMMER		(1<<1)
#define MODE_WINTER		(1<<2)

//Входы (датчики)
#define SENS_OUT		(1<<0)
#define SENS_AIR		(1<<1)
#define SENS_WATER		(1<<2)
#define SENS_EX			(1<<3)
#define SENS_ROOM		(1<<4)
#define SENS_LOC		(1<<5)
#define SENS_OUT_H		(1<<6)
#define SENS_AIR_H		(1<<7)
#define SENS_ROOM_H		(1<<8)

//Выходы
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
#define	P_STAT_RUN			0	//работа системы
#define	P_STAT_LED0			1	//индикатор красный
#define	P_STAT_LED1			2	//индикатор зелёный
#define	P_STAT_BLOCK		3	//блокировка работы
#define	P_STAT_TIMER		4	//работа по таймеру
#define	P_STAT_MENU_READY	5	//меню готово
#define	P_STAT_MENU			6	//меню открыто
#define	P_STAT_MENU_READ	7	//меню чтение
#define	P_STAT_MENU_WRITE	8	//меню запись
#define	P_STAT_SYNC			9	//синхронизация
#define	P_STAT_SEL			10	//выделение строки
#define	P_STAT_CLEAR		11	//очистка экрана
*/
/*
#define	P_STAT_RUN			0	//работа системы
#define	P_STAT_LED0			1	//индикатор красный
#define	P_STAT_LED1			2	//индикатор зелёный
#define	P_STAT_BLOCK		3	//блокировка системы
#define	P_STAT_TIMER		4	//работа по таймеру
#define	P_STAT_MENU_READY	5	//меню готово (только для записи)
#define	P_STAT_MENU			5	//меню открыто (только для чтения)
#define	P_STAT_MENU_READ	6	//меню чтение (только для чтения)
#define	P_STAT_MENU_WRITE	7	//меню запись (только для чтения)
#define	P_STAT_SYNC			8	//синхронизация (только для чтения)
#define	P_STAT_SEL			9	//выделение строки (только для чтения)
#define	P_STAT_CLEAR		10	//очистка экрана (только для чтения)
#define	P_STAT_ALARM_R		11	//сброс аварий (только для чтения)
*/

#define	P_STAT_RUN			0	//работа системы
#define	P_STAT_LED0			1	//индикатор красный
#define	P_STAT_LED1			2	//индикатор зелёный
#define	P_STAT_BLOCK		3	//блокировка системы
#define	P_STAT_TIMER		4	//работа по таймеру
#define	P_STAT_TIMER_BLOCK	5	//блокировка работы по таймеру
#define	P_STAT_MENU_READY	6	//меню готово (только для записи)
#define	P_STAT_MENU			6	//меню открыто (только для чтения)
#define	P_STAT_MENU_READ	7	//меню чтение (только для чтения)
#define	P_STAT_MENU_WRITE	8	//меню запись (только для чтения)
#define	P_STAT_SYNC			9	//синхронизация (только для чтения)
#define	P_STAT_ALARM_R		10	//сброс аварий (только для чтения)

#define	P_STAT_RUN_E		(1<<0)	//работа системы включить
#define	P_STAT_RUN_D		(1<<1)	//работа системы выключить
#define	P_STAT_TIMER_E		(1<<2)	//работа по таймеру включить
#define	P_STAT_TIMER_D		(1<<3)	//работа по таймеру выключить

//Переменные для работы с панелью
unsigned char	p_line;
mb_data 		*p_mb;

//Функции работы с панелью
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

//Переменные панели
unsigned int 	p_menu_line_en[3];	//Активные строки меню
unsigned int 	p_status;			//Статусы панели
unsigned int 	p_mode_en;			//Активные режимы
unsigned int 	p_mode;				//Текущие режимы
unsigned int 	p_speed_en;			//Активный скорости
unsigned int 	p_speed;			//Текущая скорость
unsigned int 	p_inout_en;			//Активные индикации
unsigned int 	p_inout;			//Текущие индикации
int 			p_set_t;			//Уставка температуры
unsigned int 	p_set_h;			//Уставка влажности
unsigned char	p_menu;				//Номер активного меню
