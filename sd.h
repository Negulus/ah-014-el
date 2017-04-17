#include "modbus.h"

#define SDC_WORK	(1<<0)	//работа системы
#define SDC_SPEED1	(1<<1)	//скорость 1
#define SDC_SPEED2	(1<<2)	//скорость 2
#define SDC_SPEED3	(1<<3)	//скорость 3
#define SDC_ALARM1	(1<<4)	//авария 1
#define SDC_ALARM2	(1<<5)	//авария 2
#define SDC_TERMO	(1<<6)	//термостат
#define SDC_HELP	(1<<7)	//включение резерва
#define SDC_FIRE	(1<<8)	//пожарная авария

unsigned char	sd_state;

//Регистры сд
unsigned int	sd_coils;
unsigned int	sd_reg[16];
mb_data			*sd_mb;

void sd_init(mb_data *mb);
void sd_ans();
void sd_update();
/*
typedef struct PID_DATA {
  int8_t lastProcessValue;
  int32_t sumError;
  int16_t P_Factor;
  int16_t I_Factor;
  int16_t D_Factor;
  int16_t maxError;
  int32_t maxSumError;
} pidData_t;

pidData_t pid[2];
*/
