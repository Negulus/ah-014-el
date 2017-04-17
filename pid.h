#ifndef __PID_H__
#define __PID_H__ 1

#define SCALING_FACTOR  128

typedef struct PID_DATA
{
	int		p;
	int		i;
	int		d;
	long	last;
	long	min;
	long	max;
	long	sum;
	long	sum_max;
	long	out;
} pid_data;

int		delta;
long	tmp_p;
long	tmp_i;
long	tmp_d;
long	tmp_pid;

void	pid_init(int kp, int ki, int kd, long min, long max, long sum_max, pid_data *pid);
void	pid_reset(pid_data *pid);
void	pid_calc(int setpoint, int value, pid_data *pid);

#endif
