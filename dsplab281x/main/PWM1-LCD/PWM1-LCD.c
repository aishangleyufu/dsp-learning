/*****************************************************************
**描述:利用GP定时器4的比较器在产生PWM波，控制LCD背光亮度**
**系统时钟150M，高速外设时钟25M，128分频后定时器为5.12us******
****************************************************************/
#include "DSP281x_Device.h"

void 	EVB_Timer4()
{
    EvbRegs.EXTCONB.bit.INDCOE = 1; //单独使能比较输出模式
    EvbRegs.GPTCONB.all = 0x0024;    //GP定时器4比较输出低有效
    EvbRegs.T4PR = 0x0016;           //定时周期为5.12us*(T1PR+1)
    EvbRegs.T4CMPR = 0x0008;  //GP定时器的比较寄存器，调整该值即可调整背光亮度
    EvbRegs.T4CNT = 0x0000;   //定时器初值
    EvbRegs.T4CON.all = 0x1742;      //连续增计数，128分频，使能比较，打开定时器
}

void IOinit()
{
 	EALLOW;  
 	//将GPIOB7配置为外设口
 	GpioMuxRegs.GPBMUX.bit.T4PWM_GPIOB7 = 1;
	EDIS;			
}

void main(void)
{		
   	InitSysCtrl();      //初始化系统控制寄存器, 时钟频率150M
	EALLOW;				
	SysCtrlRegs.HISPCP.all = 0x0003;//高速时钟的工作频率＝25M
	EDIS;
	DINT;	        //关闭总中断，清除中断标志
	IER = 0x0000;   //关闭外围中断
	IFR = 0x0000;	//清中断标志
	IOinit();		
	EVB_Timer4();
	for(;;){;}
} 
