
/*led��������ʵ��,�������ʱ����ѭ�����������DS4-DS19*/
#include "DSP281x_Device.h"

void LedOut(Uint16 led);
void Delay(Uint16  data);
unsigned int LedCount;
Uint16 led=0x01;	//led��ֵ 
void IOinit() //I/O�ڳ�ʼ��
{
 	EALLOW;  
 	 //��GPIOB8~GPIOB15����Ϊһ��I/O��,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff;
	 //��GPIOB8~GPIOB15����Ϊ���,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00;
    //��GPIOE0~GPIOE2����Ϊһ��I/O�����,��138���� 	  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
  	EDIS;			
}

void main(void)
{
	InitSysCtrl();	/*��ʼ��ϵͳ*/	
	DINT;  /*���ж�*/
	IER = 0x0000;
	IFR = 0x0000;
	IOinit(); //I/O�ڳ�ʼ��
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
    led =led << 1; //led��ֵ����1λ
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
    //��GPIOB8~GPIOB15����Ϊ���,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00;
	EDIS;	
	GpioDataRegs.GPEDAT.all = 0xfffb;    //LEDBѡͨ
	GpioDataRegs.GPBDAT.all = ~led;		//��ʾ��8λ
	for (i=0; i<100; i++){}              //��ʱ
    GpioDataRegs.GPEDAT.all = 0xffff;    //�����8λ	
	GpioDataRegs.GPEDAT.all = 0xfffa;    //LEDAѡͨ
	GpioDataRegs.GPBDAT.all = ~(led<<8);//��ʾ��8λ
	for (i=0; i<100; i++){}
    GpioDataRegs.GPEDAT.all = 0xffff;    //�����8λ			
}

void Delay(Uint16  data)
{
	Uint16	i;
	for (i=0;i<data;i++) { ; }	
}
//===========================================================================
// No more.
//===========================================================================

