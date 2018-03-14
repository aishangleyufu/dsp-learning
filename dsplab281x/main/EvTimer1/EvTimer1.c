/****************************************************************
**描述:利用事件管理器GP定时器1在GPIOF8~GPIOF13引脚上产生跑马灯，**
**系统时钟150M，高速外设时钟25M，128分频后定时器周期为5.12us****
****************************************************************/
#include "DSP281x_Device.h"
interrupt void eva_timer1_isr(void);
unsigned int LedCount;
const	Uint16	LedCode[]={0xFE00,0xFD00,0xFB00,0xF700,0xEF00,0xDF00,0xFF00};
void IOinit()
{
 	EALLOW;  
 	//将GPIOF8~GPIOF13配置为一般I/O口，输出
 	GpioMuxRegs.GPFMUX.all = 0xc0ff;
    GpioMuxRegs.GPFDIR.all = 0x3f00;   
  	EDIS;			
}

void 	EVA_Timer1()
{
    EvaRegs.GPTCONA.all = 0;         // 初始化 EVA Timer 1
    EvaRegs.T1PR = 0x9895;           // 定时周期为5.12us*(T1PR+1)=0.2s
    EvaRegs.EVAIMRA.bit.T1PINT = 1;  //使能定时器1的周期中断
    EvaRegs.EVAIFRA.bit.T1PINT = 1;   //写1清除定时器1的周期中断标志
    EvaRegs.T1CNT = 0x0000;
    EvaRegs.T1CON.all = 0x1740;       //连续增计数，128分频，打开定时器
}
void main(void)
{
	LedCount=0;
	InitSysCtrl();      //初始化系统控制寄存器, 时钟频率150M
	EALLOW;				
	SysCtrlRegs.HISPCP.all = 0x0003;//高速时钟的工作频率＝25M
	EDIS;
	DINT;	        //关闭总中断，清除中断标志
	IER = 0x0000;    //关闭外围中断
	IFR = 0x0000;    	//清中断标志
	InitPieCtrl();		//初始化PIE控制寄存器
	InitPieVectTable();
	IOinit();		
	EVA_Timer1();
	EALLOW;				
	
	PieVectTable.T1PINT = &eva_timer1_isr;     //中断服务程序入口地址放入中断向量表
	EDIS;              
	//依次使能各级中断：外设中相应中断位->PIE控制器->CPU
    PieCtrlRegs.PIEIER2.all = M_INT4;   //GP定时器1使能位于PIE第2组第4个，将其使能
	IER |= M_INT2;            //PIE第2组对应于CPU的可屏蔽中断2（INT2），将其使能
	EINT;   //开总中断
	for(;;){;}
} 	

interrupt void eva_timer1_isr(void)
{ 	
	GpioDataRegs.GPFDAT.all= LedCode[LedCount];
	LedCount++;
	if (LedCount>=7)	LedCount=0;

	EvaRegs.EVAIMRA.bit.T1PINT = 1;	         //使能定时器1的周期中断
    EvaRegs.EVAIFRA.bit.T1PINT = 1;		     //写1清除定时器1的周期中断标志
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;  //清零 PIEACK中的第2组中断对应位                                       
}
