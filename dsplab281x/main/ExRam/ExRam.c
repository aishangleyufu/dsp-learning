//���ܣ�/*ʵ�ֶ��ⲿ�洢�ռ�Ķ�д����*/
#include "DSP281x_Device.h"
void	InitExRam(Uint16	Start);
void	InitExRam1(Uint16	Start);
void	RamRead(Uint16	Start);

unsigned  	int  *ExRamStart = (unsigned  int *)0x100000;

void main(void)
{

	/*��ʼ��ϵͳ*/
	InitSysCtrl();

	/*���ж�*/
	DINT;
	IER = 0x0000;
	IFR = 0x0000;
   
	InitExRam(0);
	RamRead(0x4000);
	InitExRam1(0x0);		//���������öϵ�	
	for(;;);

} 	


void	InitExRam(Uint16	Start)
		{
			Uint16	i;
			for	(i=0;i<0x4000;i++)		*(ExRamStart + Start + i) = i;
		}
void	InitExRam1(Uint16	Start)
		{
			Uint16	i;
			for	(i=0;i<0x4000;i++)		*(ExRamStart + Start + i) = 0;
		}
void	RamRead(Uint16	Start)
		{
			Uint16	i;
			for	(i=0;i<0x4000;i++)		*(ExRamStart + Start + i) = *(ExRamStart +i);
		}


