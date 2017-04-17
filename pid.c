#include "main.h"
#include "pid.h"

void pid_init(int kp, int ki, int kd, long min, long max, long sum_max, pid_data *pid)
{
	pid->p = kp;
	pid->i = ki;
	pid->d = kd;
	pid->last = 0;
	pid->min = min;
	pid->max = max;
	pid->sum = 0;
	pid->sum_max = sum_max;
	pid->out = 0;
}

void pid_calc(int setpoint, int value, pid_data *pid)
{
	delta = setpoint - value;

	//-----P-----
	tmp_p = pid->p * delta;

	if (tmp_p > pid->max)
	{
		tmp_p = pid->max;
	}
	else if (tmp_p < -pid->max)
	{
		tmp_p = -pid->max;
	}

	//-----I-----
	tmp_i = pid->sum + (pid->i * delta);

	if(tmp_i > pid->sum_max)
	{
		tmp_i = pid->sum_max;
		pid->sum = pid->sum_max;
	}
	else if(tmp_i < -pid->sum_max)
	{
		tmp_i = -pid->sum_max;
		pid->sum = -pid->sum_max;
	}
	else
	{
		pid->sum = tmp_i;
	}

	//-----D-----
	tmp_d = pid->d * (pid->last - value);

	//-----Full--
	pid->last = value;
	tmp_pid = (tmp_p + tmp_i + tmp_d);

	if(tmp_pid > pid->max)
	{
		tmp_pid = pid->max;
	}
	else if(tmp_pid < pid->min)
	{
		tmp_pid = pid->min;
	}

	pid->out = tmp_pid;
}

void pid_reset(pid_data *pid)
{
	pid->sum = 0;
	pid->out = 0;
}
