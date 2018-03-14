/******************************************************************************************
*描述:spwm波形发生器,PWM1-6控制开关频率60KHz, sin表长8192(8K)幅值0～1250,频率可调范围4Hz～2KHz
*中断T1PINT更新PWM1～6的占空比，用全比较器产生3对PWM波，GP定时器1作全比较单元时基
******************************************************************************************/
#include"DSP281x_Device.h"
#include   "math.h"           //要用到正弦值的计算

static  long  x1=0;
static  long  x2=0;
static  long  x3=0;	

unsigned long int i = 0;
void  IOinit(void);

Uint16  sindata[8192];     //存正弦表
interrupt void   T1pint_isr(void);//中断子程序的声明

void  stop_pwm(void);        //停止PWM子程序
void  starlpwm(void);         //启动PWM子程序

#define   PI2   3.1416* 2      //2π
#define   FC    60000        //保存载波频率60KHz(PWM1～6)
#define   sinlen 8192           //8K的正弦表长度
long     Tlpr;              //保存n定时器的周期寄存器的值

unsigned long int    deaout=0;   //EVA输出频率
unsigned long int    dea=0;   //EVA输出频率对应查表等分值
unsigned long int    dea1=0;   //对应查表等分值中间变量
int  j;

void EVA_PWM()
{
   	EvaRegs.EXTCONA.bit.INDCOE = 1;  //单独使能比较输出模式
    EvaRegs.ACTRA.all = 0x0aaa;     //空间矢量不动作
    EvaRegs.DBTCONA.all = 0x08ec;   //死区定时器启动
    EvaRegs.CMPR1 = 0x0006;
    EvaRegs.CMPR2 = 0x0005;
    EvaRegs.CMPR3 = 0x0004;
    EvaRegs.COMCONA.all = 0xa6e0;   //空间向量禁止，全比较使能，陷阱禁止
}

void EVA_Timer1()
{
   	EvaRegs.EXTCONA.bit.INDCOE = 1;  //单独使能比较输出模式
    EvaRegs.GPTCONA.all = 0x0000;   //GP定时器1比较输出低有效
  	Tlpr=75000000/FC;
	EvaRegs.T1PR=Tlpr;   
    EvaRegs.T1CNT = 0x0000;     // 定时器初值
    EvaRegs.T1CON.all = 0x0846;//连续增减计数,1分频,使能比较,内部时钟,打开定时器
}

void main(void)
{		
    InitSysCtrl();  //初始化系统控制寄存器, 时钟频率150M,HSPCLK为75M
	DINT;	        //关闭总中断，清除中断标志
	IER = 0x0000;   //关闭外围中断
	IFR = 0x0000;	//清中断标志 	
	EVA_PWM();	
	EVA_Timer1();

	EALLOW;
	GpioMuxRegs.GPAMUX.all=0x003F;    //EVAPWMl-6
	EDIS;

//初始化PIE控制寄存器
InitPieCtrl();    
//初始化PIE中断向量表
InitPieVectTable(); 

//使能Eva相应位
	EvaRegs.COMCONA.bit.FCOMPOE=1;	//比较单元使能
	EvaRegs.EVAIMRA.bit.T1PINT=1;	//使能T1PINT中断
	EvaRegs.EVAIFRA.bit.T1PINT=1;	//先清

//．初始化占空比列表
	for(j=0;j<8192;j++)	//在线计算正弦表
	{
	sindata[j]=625+sin(j*PI2/8192)*625*0.85;
	}

//####中断使能
	EALLOW;
	PieVectTable.T1PINT=&T1pint_isr;//PIE中断矢量表	
	EDIS;

	PieCtrlRegs.PIEIER2.bit.INTx4=1;	//使能T1PINT
	IER |= M_INT2;	//使能INT2

	EINT;	//开放全局中断
	ERTM;	//EnableGlobal realtime interrupt DBGM

//主循环
// for(;;){;}
 	while (1)
 	{
	dea1=8192*deaout*2/1000;//中间变量，防计算溢出
	dea=dea1*Tlpr/75000;//计算查表等分值
	}
}

	interrupt  void  T1pint_isr(void)//中断T1PINT更新PWMl～6的占空比(60k)
{
	static  long  Ia=0;
	static  long  resa=0;

//####  更新pwm1～6占空比
	x1=Ia*dea;
	x1=resa+x1;//A相角度
	Ia++;	//时间等分
	if(x1>(sinlen-1))	//如果超出正弦表长度，则减去超出正弦表长度
	{
	resa=x1-sinlen;	//A相角度范围限制在0～360
	x1=resa;
	Ia=1;	//重新开始计数，t=0
	}
	x2=x1-2731;	//x2=x1-sinlen／3；B相
	x3=x1-5461;	//x3=xl-sinlen*2／3；C相
	if(x2<0)
	x2=x2+sinlen;	//B相角度范围限制在0～360°
	if(x3<0)	
	x3=x3+sinlen;		//C相角度范围限制在0～360°
	EvaRegs.CMPR1=sindata[x1];	//刷新占空比
	EvaRegs.CMPR2=sindata[x2];
	EvaRegs.CMPR3=sindata[x3];

	//####  复位
	EvaRegs.EVAIFRA.bit.T1PINT=1;	//中断标志清
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP2; //允许下次中断
	return;
}

