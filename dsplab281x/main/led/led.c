
/*led输出跑马灯实验,用软件延时方法循环点亮发光管DS4-DS19*/
#include "DSP281x_Device.h"

void LedOut(Uint16 led);
void Delay(Uint16  data);
unsigned int LedCount;
Uint16 led=0x01;	//led初值 
void IOinit() //I/O口初始化
{
 	EALLOW;  
 	 //将GPIOB8~GPIOB15配置为一般I/O口,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff;
	 //将GPIOB8~GPIOB15配置为输出,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00;
    //将GPIOE0~GPIOE2配置为一般I/O口输出,作138译码 	  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
  	EDIS;			
}

void main(void)
{
	InitSysCtrl();	/*初始化系统*/	
	DINT;  /*关中断*/
	IER = 0x0000;
	IFR = 0x0000;
	IOinit(); //I/O口初始化
	while (1)
	{
	 LedOut(led);
	 Delay(60000);
	 Delay(60000);
	 Delay(60000);
	 Delay(60000);
	 Delay(60000);
	 Delay(60000);
	 Delay(60000);
	 Delay(60000); 
	 Delay(60000);
	 Delay(60000);	
    led =led << 1; //led的值左移1位
	LedCount++;
	if (LedCount>=16)	
	{
	LedCount=0 ;
	led=1;
	}
 	}
}
 
void LedOut(Uint16 led)
{
 	Uint16	i;
 	EALLOW;  
    //将GPIOB8~GPIOB15配置为输出,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00;
	EDIS;	
	GpioDataRegs.GPEDAT.all = 0xfffb;    //LEDB选通
	GpioDataRegs.GPBDAT.all = ~led;		//显示高8位
	for (i=0; i<100; i++){}              //延时
    GpioDataRegs.GPEDAT.all = 0xffff;    //锁存高8位	
	GpioDataRegs.GPEDAT.all = 0xfffa;    //LEDA选通
	GpioDataRegs.GPBDAT.all = ~(led<<8);//显示低8位
	for (i=0; i<100; i++){}
    GpioDataRegs.GPEDAT.all = 0xffff;    //锁存低8位			
}

void Delay(Uint16  data)
{
	Uint16	i;
	for (i=0;i<data;i++) { ; }	
}
//===========================================================================
// No more.
//===========================================================================

