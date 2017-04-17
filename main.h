#ifndef __MAIN_H__
#define __MAIN_H__ 1

#include <avr/eeprom.h>

//Констатны для таймеров
#define	T_MB_ACCEPT	2000
#define	T_MB_ACCEPT_M	500
#define	T_MB_NEXT	25
#define	T_MB_CHECK	5000
#define T_DIN		1000
#define T_SPEED		1000

#define NO			0
#define NC			1

#define LED_GREEN	(1<<0)
#define LED_RED		(1<<1)

#define	T_MS_AC		(1<<0)
#define	T_10MS_AC	(1<<1)
#define	T_50MS_AC	(1<<2)
#define	T_S_AC		(1<<3)
#define	T_M_AC		(1<<4)
#define	T_H_AC		(1<<5)

extern unsigned int	p_menu_1[16] EEMEM;
extern unsigned int	p_menu_2[16] EEMEM;
extern unsigned int	p_day_0[12] EEMEM;
extern unsigned int	p_day_1[12] EEMEM;
extern unsigned int	p_day_2[12] EEMEM;
extern unsigned int	p_day_3[12] EEMEM;
extern unsigned int	p_day_4[12] EEMEM;
extern unsigned int	p_day_5[12] EEMEM;
extern unsigned int	p_day_6[12] EEMEM;

//Внутренние переменные
unsigned char	i;
unsigned char	j;
int				tmp;
long			tmpl;
char			tmpc;

//Функции инициализации
void mem_init();
void t_sys_init();
void ain_init();
void aout_init();
void dio_init();

//Аналоговые входы
int				ain[6];
int  			ainp;
unsigned char	ain_cnt;
unsigned char	ain_sel;
unsigned int	ainx[10];
unsigned int	nc10k_r[11];
int				nc10k_t[11];
int				ain_calc(int aint);

//Алалоговые выходы PWM
int				aout0;
int				aout1;
int				aout2;
int				aout3;

//Дискретные входы
unsigned int	din;
unsigned int	din_tmp;
unsigned int	t_din[10];
void			din_any(int port, char bit, int tmp_din, unsigned char nc);

//Дискретные выходы
unsigned char	dout;

//Индикаторы
unsigned char	led;

//Функции входов и выходов
void			io_check();

//Таймеры внутренние
unsigned int	t_ms;
unsigned char	t_10ms;
unsigned char	t_50ms;
unsigned char	t_s;
unsigned char	t_m;
unsigned char	t_h;

unsigned char t_ac;

//Таймеры системные
//unsigned int  t_pwm;

typedef struct TIMER_DATA
{
	unsigned char	stat;
	unsigned int	set;
	unsigned int	t;
} timer_data;

timer_data t_test;
timer_data t_ain;

//Функции таймеров
void timer_io_check();
void timer_ms();
void timer_10ms();
void timer_50ms();
void timer_s();
void timer_m();
void timer_h();

void timer_upd(timer_data *timer);
char timer_out(timer_data *timer);
void timer_set(timer_data *timer, unsigned int set);
void timer_start(timer_data *timer);
void timer_stop(timer_data *timer);
void timer_reset(timer_data *timer);
void timer_init(timer_data *timer);

//Данные расписания
void shed_init();					//Инициализация расписания
void shed_upd();					//Обновление переменных из памяти
void shedule();						//Работа расписания
void shed_on(unsigned char num);	//Включение системы по расписанию
void shed_off();					//Выключение системы по расписанию

unsigned int shed_day[12];

#endif
