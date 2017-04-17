#define SCL_CLOCK		200000L
#define TWI_TIME		500
#define TWI_ADR			0xA2

#define TWI_RD	1
#define TWI_WR	0

#define	TWI_LINE_C1_W	0
#define	TWI_LINE_C2_W	1
#define	TWI_LINE_SS_R	2
#define	TWI_LINE_MM_R	3
#define	TWI_LINE_HH_R	4
#define	TWI_LINE_DD_R	5
#define	TWI_LINE_WD_R	6
#define	TWI_LINE_MN_R	7
#define	TWI_LINE_YR_R	8
#define	TWI_LINE_SS_W	9
#define	TWI_LINE_MM_W	10
#define	TWI_LINE_HH_W	11
#define	TWI_LINE_DD_W	12
#define	TWI_LINE_WD_W	13
#define	TWI_LINE_MN_W	14
#define	TWI_LINE_YR_W	15

typedef struct TWI_DATA
{
	unsigned char	state;
	unsigned char	accept;
	unsigned char	func;
	unsigned char	cnt;
	unsigned char	ctu;
	unsigned char	adr;
	unsigned char	reg;
	unsigned char	data[8];
	unsigned char	line;
} twi_data;
twi_data	twi;

unsigned int t_twi;
unsigned int t_twi_set;

void twi_init();
void twi_read(unsigned char adr, unsigned char reg, unsigned char cnt);
void twi_write(unsigned char adr, unsigned char reg, unsigned char cnt);
void twi_queue();

typedef struct TIME_DATA
{
	unsigned int	state;	//State
	unsigned char	ss;		//Seconds 
	unsigned char	mm;		//Minutes 
	unsigned char	hh;		//Hours 
	unsigned char	dd;		//Days 
	unsigned char	wd;		//Weekdays 
	unsigned char	mn;		//Months
	unsigned char	cn;		//Century 
	unsigned char	yr;		//Years 
} time_data;

//Данные часов
time_data	time;
void		time_proc();

char BCD2DEC(char in);
char DEC2BCD(char in);
