#define	T_MB_ACCEPT	2000
#define	T_MB_NEXT	50
#define	T_MB_CHECK	5000
#define T_DIN		1000

//Таймеры системные
unsigned int  t_ms;
unsigned char t_s;
unsigned char t_m;
unsigned char t_h;

//Таймеры остальные

unsigned int  t_mb[2];
unsigned int  t_mb_set[2];
unsigned int  t_mb_accept[2];
unsigned int  t_mb_next[2];
unsigned int  t_mb_next_set[2];
unsigned int  t_mb_check[2];
unsigned int  t_mb_stoptx[2];
unsigned int  t_mb_stoptx_set[2];

unsigned int  t_cond;
unsigned int  t_ain;

//Функции таймеров
void timer_mb(unsigned char mbn);
void timer_mb_set(unsigned char mbn, unsigned int set);
void timer_mb_stoptx_set(unsigned char mbn, unsigned int set);
void timer_mb_accept(unsigned char mbn);
void timer_mb_check(unsigned char mbn);
void timer_mb_next(unsigned char mbn);
void timer_mb_next_set(unsigned char mbn, unsigned int set);

void timer_io_check();
