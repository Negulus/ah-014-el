#include "main.h"
#include "pid.h"

#define DOUT_SPEED1	(1<<0)
#define DOUT_SPEED2	(1<<2)
#define DOUT_SPEED3	(1<<1)
#define DOUT_DUMP	(1<<3)
#define DOUT_HEAT	(1<<4)
#define DOUT_HEAT2	(1<<5)

#define DOUT_EN		DOUT_SPEED1|DOUT_SPEED2|DOUT_SPEED3|DOUT_DUMP|DOUT_HEAT|DOUT_HEAT2

#define DIN_FIRE		(1<<0)
#define DIN_FILTER		(1<<1)
#define DIN_VENT		(1<<2)
#define DIN_DPS			(1<<3)
#define DIN_OVERHEAT	(1<<4)
#define DIN_5			(1<<5)
#define DIN_6			(1<<6)
#define DIN_7			(1<<7)
#define DIN_8			(1<<8)
#define DIN_9			(1<<9)

#define SYS_WORK	(1<<0)
#define SYS_ALARM	(1<<1)
#define SYS_BLOCK	(1<<2)
#define SYS_START	(1<<3)
#define SYS_VENT	(1<<4)
#define SYS_DUMP	(1<<5)
#define SYS_BLOW	(1<<6)
#define SYS_HEAT	(1<<7)
#define SYS_COOL	(1<<8)
#define SYS_FIRST	(1<<9)
#define SYS_HEAT2	(1<<10)

#define SYS_ER_FIRE		(1<<0)
#define SYS_ER_VENT1	(1<<1)
#define SYS_ER_VENT2	(1<<2)
#define SYS_ER_DPS1		(1<<3)
#define SYS_ER_DPS2		(1<<4)
#define SYS_ER_FILTER	(1<<5)
#define SYS_ER_OVERHEAT	(1<<6)
#define SYS_ER_FROST	(1<<7)
#define SYS_ER_AIN		(1<<8)

unsigned int	s_state;
unsigned int	s_er;
unsigned char	speed_last;

unsigned char	pid_p;
unsigned char	pid_i;
unsigned char	s_dump;
unsigned char	s_blow;
unsigned char	s_pwm;
unsigned int	s_pwm_tmp;
unsigned int	t_pwm;
timer_data 		t_dump;
timer_data 		t_speed;
timer_data 		t_blow;

pid_data		pid_heat;
char			heat_p;

void			s_init();
void			s_check();
void			s_timer_ms();
void			s_timer_10ms();
void			s_timer_50ms();
void			s_timer_s();
void			s_mem_read();
