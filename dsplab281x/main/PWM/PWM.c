/************************************************************
**描述:利用GP定时器1的比较器在产生一路PWM波，外设时钟25M**
**同时用全比较器产生3对PWM波，GP定时器1作全比较单元时基***
************************************************************/
#include "DSP281x_Device.h"

void EVA_PWM()
{
    EvaRegs.EXTCONA.bit.INDCOE = 1;  //单独使能比较输出模式
    EvaRegs.ACTRA.all = 0x0aaa;     //空间矢量不动作
    EvaRegs.DBTCONA.all = 0x08ec;   //死区定时器启动
    EvaRegs.CMPR1 = 0x0006;
    EvaRegs.CMPR2 = 0x0005;
    EvaRegs.CMPR3 = 0x0004;
    EvaRegs.COMCONA.all = 0xa4e0;   //空间向量禁止，全比较使能，陷阱禁止
}

void EVA_Timer1()
{
    EvaRegs.EXTCONA.bit.INDCOE = 1;  //单独使能比较输出模式
    EvaRegs.GPTCONA.all = 0x0012;   //GP定时器1比较输出低有效
    EvaRegs.T1PR = 0x0013;      // 定时周期为5.12us*(T1PR+1)
    EvaRegs.T1CMPR = 0x0003;    // GP定时器的比较寄存器
    EvaRegs.T1CNT = 0x0000;     // 定时器初值
    EvaRegs.T1CON.all = 0x1742;//连续增计数，128分频，使能比较，打开定时器
}

void IOinit()
{
 	EALLOW;  
 	//将GPIOA配置为外设口
 	GpioMuxRegs.GPAMUX.all = 0xffff;
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
	EVA_PWM();	
	EVA_Timer1();
	for(;;){;}
} 
